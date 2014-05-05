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

#include "mhdimagewriter.h"

#include <fstream>

#include "tgt/filesystem.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationdisk.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/tools/textfileparser.h"

/*
 * Full format specification at http://www.itk.org/Wiki/MetaIO/Documentation
 */

namespace campvis {
    const std::string MhdImageWriter::loggerCat_ = "CAMPVis.modules.io.MhdImageWriter";

    MhdImageWriter::MhdImageWriter() 
        : AbstractProcessor()
        , p_inputImage("InputImage", "Input Image", "image", DataNameProperty::READ)
        , p_fileName("FileName", "File Name", "", StringProperty::SAVE_FILENAME)
        , p_saveFile("SaveFile", "Save to File")
    {
        addProperty(p_inputImage, VALID);
        addProperty(p_fileName, VALID);
        addProperty(p_saveFile);
    }

    MhdImageWriter::~MhdImageWriter() {

    }

    void MhdImageWriter::updateResult(DataContainer& dataContainer) {
        ImageRepresentationLocal::ScopedRepresentation image(dataContainer, p_inputImage.getValue());

        if (image != 0) {
            if (image->getDimensionality() > 1 && image->getDimensionality() < 4) {
                std::string mhdName = p_fileName.getValue();
                std::string rawName = tgt::FileSystem::fullBaseName(mhdName) + ".raw";
                WeaklyTypedPointer wtp = image->getWeaklyTypedPointer();
                const ImageMappingInformation& imi = image->getParent()->getMappingInformation();

                std::fstream mhdStream(mhdName.c_str(), std::ios::out);
                std::fstream rawStream(rawName.c_str(), std::ios::out | std::ios::binary);

                if (!mhdStream.is_open() || !rawStream.is_open() || mhdStream.bad() || rawStream.bad())
                    throw tgt::IOException();

                // write MHD file
                mhdStream << "ObjectType = Image\n";
                mhdStream << "NDims = " << image->getDimensionality() << "\n";

                mhdStream << "DimSize = " << image->getSize().x << " " << image->getSize().y;
                if (image->getDimensionality() > 2)
                    mhdStream << " " << image->getSize().z;
                mhdStream << "\n";

                mhdStream << "ElementSpacing = " << imi.getVoxelSize().x << " " << imi.getVoxelSize().y;
                if (image->getDimensionality() > 2)
                    mhdStream << " " << imi.getVoxelSize().z;
                mhdStream << "\n";

                mhdStream << "Position = " << imi.getOffset().x << " " << imi.getOffset().y;
                if (image->getDimensionality() > 2)
                    mhdStream << " " << imi.getOffset().z;
                mhdStream << "\n";

                switch (wtp._baseType) {
                    case WeaklyTypedPointer::UINT8:
                        mhdStream << "ElementType = MET_UCHAR\n";
                        break;
                    case WeaklyTypedPointer::INT8:
                        mhdStream << "ElementType = MET_CHAR\n";
                        break;
                    case WeaklyTypedPointer::UINT16:
                        mhdStream << "ElementType = MET_USHORT\n";
                        break;
                    case WeaklyTypedPointer::INT16:
                        mhdStream << "ElementType = MET_SHORT\n";
                        break;
                    case WeaklyTypedPointer::UINT32:
                        mhdStream << "ElementType = MET_UINT\n";
                        break;
                    case WeaklyTypedPointer::INT32:
                        mhdStream << "ElementType = MET_INT\n";
                        break;
                    case WeaklyTypedPointer::FLOAT:
                        mhdStream << "ElementType = MET_FLOAT\n";
                        break;
                    default:
                        tgtAssert(false, "Should not reach this - wrong base data type!");
                        break;
                }

                mhdStream << "ElementNumberOfChannels = " << image->getParent()->getNumChannels() << "\n";
                mhdStream << "ElementByteOrderMSB = False\n";
                mhdStream << "ElementDataFile = " << tgt::FileSystem::fileName(rawName) << "\n";

                // write raw file
                const char* rawData = static_cast<const char*>(wtp._pointer);
                size_t numBytes = wtp.getNumBytesPerElement() * image->getNumElements();
                rawStream.write(rawData, numBytes);

                if (mhdStream.bad() || rawStream.bad())
                    throw tgt::IOException();

                mhdStream.close();
                rawStream.close();
            }
            else {
                LERROR("MHD only supports 2D or 3D images.");
            }
        }
        else {
            LERROR("Could not get Image to write from DataContainer.");
        }

        validate(INVALID_RESULT);
    }

}