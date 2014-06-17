// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#include "itkreader.h"

#include "modules/itk/core/genericimagerepresentationitk.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"



namespace campvis {

    const std::string ItkReader::loggerCat_ = "CAMPVis.modules.io.ItkReader";

    ItkReader::ItkReader()
        : AbstractProcessor()
        , p_url("sourcefile", "Source File Name", "", StringProperty::OPEN_FILENAME)
        , p_targetImageID("OutoutImage", "Image Output Name", "itkImage", DataNameProperty::WRITE)
    {
        addProperty(p_url, INVALID_RESULT);
        addProperty(p_targetImageID, INVALID_RESULT);
    }

    ItkReader::~ItkReader() {

    }


    void ItkReader::updateResult(DataContainer& data)  {

        typedef itk::ImageIOBase::IOComponentType ScalarPixelType;

        itk::ImageIOBase::Pointer imageIO =
            itk::ImageIOFactory::CreateImageIO(p_url.getValue().c_str(), itk::ImageIOFactory::ReadMode);

        if (imageIO.IsNotNull())
        {
            WeaklyTypedPointer wtp;

            imageIO->SetFileName(p_url.getValue());
            imageIO->ReadImageInformation();

            const ScalarPixelType pixelType = imageIO->GetComponentType();
            const size_t numDimensions = imageIO->GetNumberOfDimensions();

            LDEBUG("Reading Image with Reader " << imageIO->GetNameOfClass());
            LDEBUG("Pixel Type is " << imageIO->GetComponentTypeAsString(pixelType));
            LDEBUG("numDimensions: " << numDimensions);

            if (numDimensions > 3) {
                LERROR("Error: Dimensions higher than 3 not supported!");
                return;
            }

            itk::ImageIORegion ioRegion(numDimensions);
            itk::ImageIORegion::IndexType ioStart = ioRegion.GetIndex();
            itk::ImageIORegion::SizeType ioSize = ioRegion.GetSize();

            tgt::vec3 imageOffset(0.f);
            tgt::vec3 voxelSize(1.f);
            tgt::ivec3 size_i(1);

            //we assured above that numDimensions is < 3
            for (int i = 0; i < numDimensions; i++) {
                size_i[i] = imageIO->GetDimensions(i);
                imageOffset[i] = imageIO->GetOrigin(i);
                voxelSize[i] = imageIO->GetSpacing(i);
                ioStart[i] = 0;
                ioSize[i] = size_i[i];
            }

            tgt::svec3 size(size_i);
            size_t dimensionality = (size_i[2] == 1) ? ((size_i[1] == 1) ? 1 : 2) : 3;

            LDEBUG("Image Size is " << size);
            LDEBUG("Voxel Size is " << voxelSize);
            LDEBUG("Image Offset is " << imageOffset);
            LDEBUG("component size: " << imageIO->GetComponentSize());
            LDEBUG("components: " << imageIO->GetNumberOfComponents());
            LDEBUG("pixel type (string): " << imageIO->GetPixelTypeAsString(imageIO->GetPixelType())); // 'vector'
            LDEBUG("pixel type: " << imageIO->GetPixelType()); // '5'

            switch (pixelType) {
            case ScalarPixelType::CHAR:
                wtp._baseType = WeaklyTypedPointer::INT8; break;
            case ScalarPixelType::UCHAR:
                wtp._baseType = WeaklyTypedPointer::UINT8; break;
            case ScalarPixelType::SHORT:
                wtp._baseType = WeaklyTypedPointer::INT16; break;
            case ScalarPixelType::USHORT:
                wtp._baseType = WeaklyTypedPointer::UINT16; break;
            case ScalarPixelType::INT:
                wtp._baseType = WeaklyTypedPointer::INT32; break;
            case ScalarPixelType::UINT:
                wtp._baseType = WeaklyTypedPointer::UINT32; break;
            case ScalarPixelType::FLOAT:
                wtp._baseType = WeaklyTypedPointer::FLOAT; break;
            default:
                LERROR("Error while loading ITK image: unsupported type: " << pixelType);
                return;
            }

            wtp._numChannels = imageIO->GetNumberOfComponents();

            //Setup the image region to read
            ioRegion.SetIndex(ioStart);
            ioRegion.SetSize(ioSize);
            imageIO->SetIORegion(ioRegion);

            //Finally, allocate buffer and read the image data
            wtp._pointer = new uint8_t[imageIO->GetImageSizeInBytes()];
            imageIO->Read(wtp._pointer);

            ImageData* image = new ImageData(dimensionality, size, wtp._numChannels);
            ImageRepresentationLocal::create(image, wtp);

            image->setMappingInformation(ImageMappingInformation(size, imageOffset/* + p_imageOffset.getValue()*/, voxelSize /** p_voxelSize.getValue()*/));
            data.addData(p_targetImageID.getValue(), image);
        }
        else {
            LWARNING("Unable to create ImageIO Instance; No suitable reader found!");
        }
        validate(INVALID_RESULT);
    }

}
