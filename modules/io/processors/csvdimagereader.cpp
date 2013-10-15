// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#include "csvdimagereader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "tgt/filesystem.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationdisk.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/tools/textfileparser.h"

namespace campvis {
    const std::string CsvdImageReader::loggerCat_ = "CAMPVis.modules.io.CsvdImageReader";

    CsvdImageReader::CsvdImageReader() 
        : AbstractProcessor()
        , p_url("url", "Image URL", "")
        , p_targetImageID("targetImageName", "Target Image ID", "CsvdImageReader.output", DataNameProperty::WRITE)
        , p_imageOffset("ImageOffset", "Image Offset in mm", tgt::vec3(0.f), tgt::vec3(-10000.f), tgt::vec3(10000.f), tgt::vec3(0.1f))
        , p_voxelSize("VoxelSize", "Voxel Size in mm", tgt::vec3(1.f), tgt::vec3(-100.f), tgt::vec3(100.f), tgt::vec3(0.1f))
    {
        addProperty(&p_url);
        addProperty(&p_targetImageID);
        addProperty(&p_imageOffset);
        addProperty(&p_voxelSize);
    }

    CsvdImageReader::~CsvdImageReader() {

    }

    void CsvdImageReader::process(DataContainer& data) {
        try {
            // start parsing
            TextFileParser tfp(p_url.getValue(), true, "=");
            tfp.parse<TextFileParser::ItemSeparatorLines>();

            // init optional parameters with sane default values
            size_t dimensionality = 3;
            tgt::svec3 size;
            WeaklyTypedPointer::BaseType pt;
            size_t numChannels = 1;

            tgt::vec3 voxelSize(1.f);
            tgt::vec3 imageOffset(0.f);

            // dimensionality and size
            if (tfp.hasKey("Size")) {
                size = tfp.getSvec3("Size");
            }
            else {
                LERROR("Error while parsing CSVD header: No Size specified.");
                validate(INVALID_RESULT);
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
                validate(INVALID_RESULT);
                return;
            }

            // dimensionality and size
            if (tfp.hasKey("CsvFileBaseName")) {
                ImageData* image = new ImageData(dimensionality, size, 1);
                ImageRepresentationLocal* rep = 0;
                size_t index = 0;

                std::string url = StringUtils::trim(tfp.getString("CsvFileBaseName"));
                url = tgt::FileSystem::cleanupPath(tgt::FileSystem::dirName(p_url.getValue()) + "/" + url);

                // start parsing of CSV files
#define DISPATCH_PARSING(WTP_TYPE, C_TYPE, TMP_TYPE) \
    if (pt == WTP_TYPE) {\
        C_TYPE* dataArray = new C_TYPE[tgt::hmul(size)]; \
        memset(dataArray, 0, sizeof(C_TYPE) * tgt::hmul(size)); \
        for (size_t slice = 0; slice < size.z; ++slice) { \
            std::stringstream ss; \
            ss << url << slice << ".csv"; \
            std::string concatenated = ss.str(); \
             \
            std::ifstream file(concatenated.c_str(), std::ifstream::in); \
            if (!file.is_open() || file.bad()) \
                throw tgt::FileException("Could not open file " + ss.str() + " for reading.", p_url.getValue()); \
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
                    throw tgt::FileException("Error while parsing the data.", p_url.getValue());
                }
            }
            else {
                LERROR("Error while parsing CSVD header: No file names specified.");
                validate(INVALID_RESULT);
                return;
            }
        }
        catch (tgt::Exception& e) {
            LERROR("Error while parsing MHD header: " << e.what());
            validate(INVALID_RESULT);
            return;
        }
        catch (std::exception& e) {
            LERROR("Error while parsing MHD header: " << e.what());
            validate(INVALID_RESULT);
            return;
        }

        validate(INVALID_RESULT);
    }
}