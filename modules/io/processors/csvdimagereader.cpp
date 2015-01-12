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

#include "csvdimagereader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "cgt/filesystem.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationdisk.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/tools/textfileparser.h"

namespace campvis {
    const std::string CsvdImageReader::loggerCat_ = "CAMPVis.modules.io.CsvdImageReader";

    CsvdImageReader::CsvdImageReader() 
        : AbstractImageReader()
        , p_imageOffset("ImageOffset", "Image Offset in mm", cgt::vec3(0.f), cgt::vec3(-10000.f), cgt::vec3(10000.f), cgt::vec3(0.1f))
        , p_voxelSize("VoxelSize", "Voxel Size in mm", cgt::vec3(1.f), cgt::vec3(-100.f), cgt::vec3(100.f), cgt::vec3(0.1f))
    {
        this->_ext.push_back("csv");
        this->p_targetImageID.setValue("CsvdImageReader.output");

        addProperty(p_url);
        addProperty(p_targetImageID);
        addProperty(p_imageOffset);
        addProperty(p_voxelSize);
    }

    CsvdImageReader::~CsvdImageReader() {

    }

    void CsvdImageReader::updateResult(DataContainer& data) {
        try {
            // start parsing
            TextFileParser tfp(p_url.getValue(), true, "=");
            tfp.parse<TextFileParser::ItemSeparatorLines>();

            // init optional parameters with sane default values
            cgt::svec3 size;
            WeaklyTypedPointer::BaseType pt;

            cgt::vec3 voxelSize(1.f);
            cgt::vec3 imageOffset(0.f);

            // dimensionality and size
            if (tfp.hasKey("Size")) {
                size = tfp.getSvec3("Size");
            }
            else {
                LERROR("Error while parsing CSVD header: No Size specified.");
                return;
            }

            // element type
            std::string et = tfp.getString("ElementType");
            if (et == "UINT8")
                pt = WeaklyTypedPointer::UINT8;
            else if (et == "INT8")
                pt = WeaklyTypedPointer::INT8;
            else if (et == "UINT16")
                pt = WeaklyTypedPointer::UINT16;
            else if (et == "INT16")
                pt = WeaklyTypedPointer::INT16;
            else if (et == "UINT32")
                pt = WeaklyTypedPointer::UINT32;
            else if (et == "INT32")
                pt = WeaklyTypedPointer::INT32;
            else if (et == "FLOAT")
                pt = WeaklyTypedPointer::FLOAT;
            else {
                LERROR("Error while parsing MHD header: Unsupported element type: " << et);
                return;
            }

            // dimensionality and size
            if (tfp.hasKey("CsvFileBaseName")) {
                size_t dimensionality = 3;
                ImageData* image = new ImageData(dimensionality, size, 1);
                ImageRepresentationLocal* rep = 0;
                size_t index = 0;

                std::string url = StringUtils::trim(tfp.getString("CsvFileBaseName"));
                url = cgt::FileSystem::cleanupPath(cgt::FileSystem::dirName(p_url.getValue()) + "/" + url);

                // start parsing of CSV files
#define DISPATCH_PARSING(WTP_TYPE, C_TYPE, TMP_TYPE) \
    if (pt == WTP_TYPE) {\
        C_TYPE* dataArray = new C_TYPE[cgt::hmul(size)]; \
        memset(dataArray, 0, sizeof(C_TYPE) * cgt::hmul(size)); \
        for (size_t slice = 0; slice < size.z; ++slice) { \
            std::stringstream ss; \
            ss << url << slice << ".csv"; \
            std::string concatenated = ss.str(); \
             \
            std::ifstream file(concatenated.c_str(), std::ifstream::in); \
            if (!file.is_open() || file.bad()) \
                throw cgt::FileException("Could not open file " + ss.str() + " for reading.", p_url.getValue()); \
                 \
            TMP_TYPE tmp; \
            for (size_t column = 0; column < size.y && file.good(); ++column) { \
                for (size_t row = 0; row < size.x && file.good(); ++row) { \
                    file >> tmp; \
                    dataArray[index++] = static_cast<C_TYPE>(tmp); \
                    file.get(); /* TODO: simple hack to advance to next character - but there might be more than one... */ \
                } \
            } \
            \
            file.close(); \
        } \
        rep = GenericImageRepresentationLocal<C_TYPE, 1>::create(image, dataArray); \
        if (rep == 0) \
            delete [] dataArray; \
    }

                DISPATCH_PARSING(WeaklyTypedPointer::UINT8      , uint8_t, uint16_t)
                else DISPATCH_PARSING(WeaklyTypedPointer::INT8  , int8_t, int16_t)
                else DISPATCH_PARSING(WeaklyTypedPointer::UINT16, uint16_t, uint16_t)
                else DISPATCH_PARSING(WeaklyTypedPointer::INT16 , int16_t, int16_t)
                else DISPATCH_PARSING(WeaklyTypedPointer::UINT32, uint32_t, uint32_t)
                else DISPATCH_PARSING(WeaklyTypedPointer::INT32 , int32_t, int32_t)
                else DISPATCH_PARSING(WeaklyTypedPointer::FLOAT , float, float)
                
                if (rep != 0) {
                    // all parsing done - lets create the image:
                    image->setMappingInformation(ImageMappingInformation(size, imageOffset + p_imageOffset.getValue(), voxelSize + p_voxelSize.getValue()));
                    data.addData(p_targetImageID.getValue(), image);
                }
                else {
                    throw cgt::FileException("Error while parsing the data.", p_url.getValue());
                }
            }
            else {
                LERROR("Error while parsing CSVD header: No file names specified.");
                return;
            }
        }
        catch (cgt::Exception& e) {
            LERROR("Error while parsing MHD header: " << e.what());
            return;
        }
        catch (std::exception& e) {
            LERROR("Error while parsing MHD header: " << e.what());
            return;
        }
    }

}