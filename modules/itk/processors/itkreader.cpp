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

#include "core/tools/stringutils.h"
#include <iomanip>

#include <itkImage.h>
#include <itkImageFileReader.h>

namespace campvis {

    const std::string ItkReader::loggerCat_ = "CAMPVis.modules.io.ItkReader";

    ItkReader::ItkReader()
        : AbstractProcessor()
        , p_url("sourcefile", "Source File", "", StringProperty::OPEN_FILENAME)
        , p_imageSeries("imageseries", "Image Series", false)
        , p_lastUrl("lastsourcefile", "Last Series File", "", StringProperty::OPEN_FILENAME)
        , p_targetImageID("OutoutImage", "Image Output Name", "itkImage", DataNameProperty::WRITE)
    {
        addProperty(p_url, INVALID_RESULT);
        addProperty(p_imageSeries, INVALID_RESULT);
        addProperty(p_lastUrl, INVALID_RESULT);
        addProperty(p_targetImageID, INVALID_RESULT);
    }

    ItkReader::~ItkReader() {

    }


    void ItkReader::updateResult(DataContainer& data)  {
        if (p_imageSeries.getValue()) {
            ReadImageSeries(data);
        }
        else {
            ReadImageDirect(data);
        }
    }

    void ItkReader::ReadImageDirect(DataContainer& data) {
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
            for (int i = 0; i < static_cast<int>(numDimensions); i++) {
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
            case itk::ImageIOBase::CHAR:
                wtp._baseType = WeaklyTypedPointer::INT8; break;
            case itk::ImageIOBase::UCHAR:
                wtp._baseType = WeaklyTypedPointer::UINT8; break;
            case itk::ImageIOBase::SHORT:
                wtp._baseType = WeaklyTypedPointer::INT16; break;
            case itk::ImageIOBase::USHORT:
                wtp._baseType = WeaklyTypedPointer::UINT16; break;
            case itk::ImageIOBase::INT:
                wtp._baseType = WeaklyTypedPointer::INT32; break;
            case itk::ImageIOBase::UINT:
                wtp._baseType = WeaklyTypedPointer::UINT32; break;
            case itk::ImageIOBase::DOUBLE:
                LWARNING("Pixel Type is DOUBLE. Conversion to float may result in loss of precision!");
            case itk::ImageIOBase::FLOAT:
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

            if (pixelType != itk::ImageIOBase::DOUBLE) {
                //Finally, allocate buffer and read the image data
                wtp._pointer = new uint8_t[imageIO->GetImageSizeInBytes()];
                imageIO->Read(wtp._pointer);
            }
            else {
                //convert float volume to double volume
                double * inputBuf = new double[imageIO->GetImageSizeInComponents()];
                wtp._pointer = new uint8_t[imageIO->GetImageSizeInComponents() * sizeof(float)];
                imageIO->Read(inputBuf);

                double * dptr = inputBuf;
                float * fptr = static_cast<float*>(wtp._pointer);
                for (int i = 0, s = imageIO->GetImageSizeInComponents(); i < s; ++i) {
                    *fptr = *dptr;
                    fptr++;
                    dptr++;
                }
                delete[] inputBuf;
            }

            ImageData* image = new ImageData(dimensionality, size, wtp._numChannels);
            ImageRepresentationLocal::create(image, wtp);

            image->setMappingInformation(ImageMappingInformation(size, imageOffset/* + p_imageOffset.getValue()*/, voxelSize /** p_voxelSize.getValue()*/));
            data.addData(p_targetImageID.getValue(), image);
        }
        else {
            LWARNING("Unable to create ImageIO Instance; No suitable reader found!");
        }
    }


    void ItkReader::ReadImageSeries(DataContainer& data) {
        typedef itk::ImageIOBase::IOComponentType ScalarPixelType;

        std::vector<std::string> imageFileNames = GetImageFileNames();

        if (!imageFileNames.size())
            return;

        itk::ImageIOBase::Pointer imageIO =
            itk::ImageIOFactory::CreateImageIO(imageFileNames[0].c_str(), itk::ImageIOFactory::ReadMode);

        const int numSlices = imageFileNames.size();

        if (imageIO.IsNotNull())
        {
            WeaklyTypedPointer wtp;

            imageIO->SetFileName(imageFileNames[0]);
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
            for (int i = 0; i < static_cast<int>(numDimensions); i++) {
                size_i[i] = imageIO->GetDimensions(i);
                imageOffset[i] = imageIO->GetOrigin(i);
                voxelSize[i] = imageIO->GetSpacing(i);
                ioStart[i] = 0;
                ioSize[i] = size_i[i];
            }

            tgt::svec3 size(size_i);
            size_t dimensionality = (size_i[2] == 1) ? ((size_i[1] == 1) ? 1 : 2) : 3;
            if (dimensionality > 2) {
                LERROR("Error: Cannot load image series with more than two dimensions!");
                return;
            }

            LDEBUG("Image Size is " << size);
            LDEBUG("Voxel Size is " << voxelSize);
            LDEBUG("Image Offset is " << imageOffset);
            LDEBUG("component size: " << imageIO->GetComponentSize());
            LDEBUG("components: " << imageIO->GetNumberOfComponents());
            LDEBUG("pixel type (string): " << imageIO->GetPixelTypeAsString(imageIO->GetPixelType()));
            LDEBUG("pixel type: " << imageIO->GetPixelType());

            switch (pixelType) {
            case itk::ImageIOBase::CHAR:
                wtp._baseType = WeaklyTypedPointer::INT8; break;
            case itk::ImageIOBase::UCHAR:
                wtp._baseType = WeaklyTypedPointer::UINT8; break;
            case itk::ImageIOBase::SHORT:
                wtp._baseType = WeaklyTypedPointer::INT16; break;
            case itk::ImageIOBase::USHORT:
                wtp._baseType = WeaklyTypedPointer::UINT16; break;
            case itk::ImageIOBase::INT:
                wtp._baseType = WeaklyTypedPointer::INT32; break;
            case itk::ImageIOBase::UINT:
                wtp._baseType = WeaklyTypedPointer::UINT32; break;
            case itk::ImageIOBase::DOUBLE:
                LWARNING("Pixel Type is DOUBLE. Conversion to float may result in loss of precision!");
            case itk::ImageIOBase::FLOAT:
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

            //allocate a temporary buffer if necessary
            double* inputBuf = (pixelType == itk::ImageIOBase::DOUBLE) ? new double[imageIO->GetImageSizeInComponents()] : nullptr;
            size_t sliceSize = (pixelType == itk::ImageIOBase::DOUBLE) ? imageIO->GetImageSizeInComponents() * sizeof(float) : imageIO->GetImageSizeInBytes();
            wtp._pointer = new uint8_t[numSlices * sliceSize];
            for (int idx = 0; idx < numSlices; ++idx) {
                itk::ImageIOBase::Pointer fileIO = imageIO;
                    //itk::ImageIOFactory::CreateImageIO(imageFileNames[idx].c_str(), itk::ImageIOFactory::ReadMode);
                fileIO->SetFileName(imageFileNames[idx]);
                fileIO->ReadImageInformation();
                fileIO->SetIORegion(ioRegion);

                size_t currentSliceSize = (pixelType == itk::ImageIOBase::DOUBLE) ? imageIO->GetImageSizeInComponents() * sizeof(float) : fileIO->GetImageSizeInBytes();
                if (currentSliceSize != sliceSize) {
                    LERROR("Image " << imageFileNames[idx] << " has different dimensionality or data type!");
                    delete static_cast<uint8_t*>(wtp._pointer);
                    delete inputBuf;
                    wtp._pointer = nullptr;
                    return;
                }

                uint8_t* sliceBuffer = static_cast<uint8_t*>(wtp._pointer) + idx * sliceSize;

                if (pixelType != itk::ImageIOBase::DOUBLE) {
                    // directly read slice into buffer
                    fileIO->Read(sliceBuffer);
                }
                else {
                    //convert float volume to double volume
                    fileIO->Read(inputBuf);

                    double* dptr = inputBuf;
                    float* fptr = reinterpret_cast<float*>(sliceBuffer);
                    for (int i = 0, s = fileIO->GetImageSizeInComponents(); i < s; ++i) {
                        *fptr = static_cast<float>(*dptr);
                        fptr++;
                        dptr++;
                    }
                }
            }
            delete[] inputBuf;

            size[2] = numSlices;
            //series adds one dimension
            ImageData* image = new ImageData(dimensionality+1, size, wtp._numChannels);
            ImageRepresentationLocal::create(image, wtp);

            image->setMappingInformation(ImageMappingInformation(size, imageOffset/* + p_imageOffset.getValue()*/, voxelSize /** p_voxelSize.getValue()*/));
            data.addData(p_targetImageID.getValue(), image);
        }
        else {
            LWARNING("Unable to create ImageIO Instance; No suitable reader found!");
        }
    }


    std::vector<std::string> ItkReader::GetImageFileNames() {
        std::vector<std::string> filenames;

        std::string first = p_url.getValue();
        std::string last = p_lastUrl.getValue();

        //either one is empty - we cant do anything
        if (!first.size() || !last.size())
            return filenames;

        //this is a pretty naive scheme to find out the naming convention of the files
        //we first scan both filenames from left to find the first position where they diverge
        size_t diverge_left = 0;
        for (; diverge_left < first.size(); ++diverge_left) {
            //check if we are outside last string
            if (diverge_left >= last.size())
                break;

            if (first[diverge_left] != last[diverge_left]) break;
        }
        std::string fileBegin = first.substr(0, diverge_left);

        //strings are equal?
        if (diverge_left == first.size() && diverge_left == last.size()) {
            LWARNING("First and last filenames are equal!");
            filenames.push_back(first);
            return filenames;
        }


        // now we assume the diverging letters are a number, which we skip forward
        int digitFirstEnd = diverge_left;
        if (!isdigit(first[digitFirstEnd])) {
            LERROR("Cannot find sequence between first and last filename!");
            return filenames;
        }
        while (isdigit(first[digitFirstEnd])) digitFirstEnd++;
        int digitLastEnd = diverge_left;
        if (!isdigit(last[digitLastEnd])) {
            LERROR("Cannot find sequence between first and last filename!");
            return filenames;
        }
        while (isdigit(last[digitLastEnd])) digitLastEnd++;

        
        std::string fileEnd = first.substr(digitFirstEnd, std::string::npos);
        if (fileEnd != last.substr(digitLastEnd, std::string::npos)) {
            LERROR("Filename Tails mismatch!");
            return filenames;
        }

        //now, we have diverge_left and diverge_right
        LINFO("Diverge Left: " << diverge_left << "; digit end position: " << digitFirstEnd);
        LINFO("String begin: " << fileBegin << "; end: " << fileEnd);

        //the substrings of first and last filename are converted to numbers to get the numerical range
        int numFirst = StringUtils::fromString<int>(first.substr(diverge_left, digitFirstEnd - diverge_left));
        int numLast = StringUtils::fromString<int>(last.substr(diverge_left, digitLastEnd - diverge_left));

        LINFO("Indices from " << first.substr(diverge_left, digitFirstEnd - diverge_left) << "(" << numFirst << ") to " << numLast);


        //then we step through all numbers in the range and generate filenames by replacing the diverged substring
        //with the generated number
        for (int imgIdx = numFirst; imgIdx <= numLast; ++imgIdx) {
            std::stringstream s;
            if (first.size() == last.size()) {
                s << std::setfill('0');
                s << std::setw(digitFirstEnd - diverge_left);
            }
            s.clear();
            s << imgIdx;
            filenames.push_back(fileBegin + s.str() + fileEnd);
            //LINFO(filenames.back());
        }


        return filenames;
    }
}
