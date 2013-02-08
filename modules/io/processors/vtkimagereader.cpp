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

#include "vtkimagereader.h"

#include <iostream>
#include <fstream>
#include <sstream>
#include <string>

#include "tgt/filesystem.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationdisk.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/tools/stringutils.h"

/*
 * Full format specification at http://www.vtk.org/VTK/img/file-formats.pdf
 */

namespace campvis {
    const std::string VtkImageReader::loggerCat_ = "CAMPVis.modules.io.VtkImageReader";

    VtkImageReader::VtkImageReader() 
        : AbstractProcessor()
        , p_url("url", "Image URL", "")
        , p_targetImageID("targetImageName", "Target Image ID", "VtkImageReader.output", DataNameProperty::WRITE)
        , p_imageOffset("ImageOffset", "Image Offset in mm", tgt::vec3(0.f), tgt::vec3(-10000.f), tgt::vec3(10000.f))
        , p_voxelSize("VoxelSize", "Voxel Size in mm", tgt::vec3(1.f), tgt::vec3(-100.f), tgt::vec3(100.f))
    {
        addProperty(&p_url);
        addProperty(&p_targetImageID);
        addProperty(&p_imageOffset);
        addProperty(&p_voxelSize);
    }

    VtkImageReader::~VtkImageReader() {

    }

    std::string getTrimmedLine(std::ifstream& file, char delim = '\n') {
        std::string toReturn;
        std::getline(file, toReturn, delim);
        return StringUtils::trim(toReturn);
    }

    void VtkImageReader::process(DataContainer& data) {
        try {
            std::ifstream file(p_url.getValue().c_str(), std::ifstream::in);
            if (!file.is_open() || file.bad())
                throw tgt::FileException("Could not open file " + p_url.getValue() + " for reading.", p_url.getValue());

            std::string curLine = getTrimmedLine(file);
            if (curLine.find("# vtk DataFile Version") != 0)
                throw tgt::FileException("Unknown identifier in vtk file.", p_url.getValue());

            // init optional parameters with sane default values
            bool binary = false;
            std::string url;
            size_t dimensionality = 3;
            tgt::svec3 size(0U);

            tgt::vec3 voxelSize(1.f);
            tgt::vec3 imageOffset(0.f);

            // next line is the header - contains only unimportant data
            curLine = getTrimmedLine(file);

            // this line is the format
            curLine = StringUtils::lowercase(getTrimmedLine(file));
            if (curLine == "ascii")
                binary = false;
            else if (curLine == "binary")
                binary = true;
            else
                throw tgt::FileException("Unsupported format in vtk file - expected binary or ascii.", p_url.getValue());

            // now comes the dataset structure
            curLine = StringUtils::lowercase(getTrimmedLine(file));
            std::vector<std::string> splitted = StringUtils::split(curLine, " ");
            if (splitted.size() != 2 || splitted[0] != "dataset" || splitted[1] != "structured_points")
                throw tgt::FileException("Unsupported dataset structure in vtk file - expected \"DATASET STRUCTURED_POINTS\".", p_url.getValue());

            // now comes dimensions, spacing and origin:
            for (size_t i = 0; i < 3; ++i) {
                std::stringstream ss(StringUtils::lowercase(getTrimmedLine(file)));
                ss >> curLine;
                if (curLine == "dimensions") {
                    ss >> size.x >> size.y >> size.z;
                    if (size.z == 1)
                        dimensionality = 2;
                    if (size.y == 1)
                        dimensionality = 1;
                }
                else if (curLine == "spacing") {
                    ss >> voxelSize.x >> voxelSize.y >> voxelSize.z;
                }
                else if (curLine == "origin") {
                    ss >> imageOffset.x >> imageOffset.y >> imageOffset.z;
                }
                else {
                    throw tgt::FileException("Unsupported dataset structure field '" + curLine + "' in vtk file.", p_url.getValue());
                }
            }

            // now come the dataset attributes "POINT_DATA ..."
            curLine = StringUtils::lowercase(getTrimmedLine(file));
            splitted = StringUtils::split(curLine, " ");
            if (splitted.size() != 2 || splitted[0] != "point_data")
                throw tgt::FileException("Unsupported dataset attribute '" + splitted[0] + "' in vtk file - expected \"POINT_DATA n\".", p_url.getValue());
            size_t numPoints = StringUtils::fromString<size_t>(splitted[1]);

            if (numPoints < tgt::hmul(size))
                throw tgt::FileException("Number of points in dataset (" + splitted[0] + ") doesn't match dimensions: " + StringUtils::toString(size), p_url.getValue());

            // now comes the data description block "FIELD ..."
            curLine = StringUtils::lowercase(getTrimmedLine(file));
            splitted = StringUtils::split(curLine, " ");
            if (splitted.size() != 3 || splitted[0] != "field")
                throw tgt::FileException("Unsupported dataset attribute '" + splitted[0] + "' in vtk file - expected \"FIELD ...\".", p_url.getValue());
            size_t numArrays = StringUtils::fromString<size_t>(splitted[2]);

            if (numArrays != 1)
                throw tgt::FileException("Multiple arrays in data set currently not supported - too lazy...", p_url.getValue());

            curLine = StringUtils::lowercase(getTrimmedLine(file));
            splitted = StringUtils::split(curLine, " ");
            size_t numTuples = StringUtils::fromString<size_t>(splitted[1]);
            size_t numComponents = StringUtils::fromString<size_t>(splitted[2]);

            if (numTuples * numComponents != numPoints)
                throw tgt::FileException("Number of points in dataset doesn't match dimensions of data field", p_url.getValue());

            if (! binary) {
                ImageData* image = new ImageData(dimensionality, size, 1);
                ImageRepresentationLocal* rep = 0;

#define DISPATCH_PARSING(VTK_TYPE, C_TYPE, TMP_TYPE) \
    do { \
        if (splitted[3] == VTK_TYPE) { \
            C_TYPE* dataArray = new C_TYPE[numPoints]; \
            TMP_TYPE tmp; \
            for (size_t i = 0; i < numPoints && file.good(); ++i) { \
                file >> tmp; \
                dataArray[i] = static_cast<C_TYPE>(tmp); \
            } \
            rep = GenericImageRepresentationLocal<C_TYPE, 1>::create(image, dataArray); \
        } \
    } while (0)

                DISPATCH_PARSING("unsigned_char"    , uint8_t, uint16_t);
                DISPATCH_PARSING("char"             , int8_t, int16_t);
                DISPATCH_PARSING("unsigned_short"   , uint16_t, uint16_t);
                DISPATCH_PARSING("short"            , int16_t, int16_t);
                DISPATCH_PARSING("unsigned_int"     , uint32_t, uint32_t);
                DISPATCH_PARSING("int"              , int32_t, int32_t);
                DISPATCH_PARSING("float"            , float, float);

                if (rep != 0) {
                    // all parsing done - lets create the image:
                    image->setMappingInformation(ImageMappingInformation(size, imageOffset + p_imageOffset.getValue(), voxelSize + p_voxelSize.getValue()));
                    data.addData(p_targetImageID.getValue(), image);
                    p_targetImageID.issueWrite();
                }
                else {
                    throw tgt::FileException("Error while parsing the data.", p_url.getValue());
                }
            }
            else {
                throw tgt::FileException("Binary data format currently unsupported.", p_url.getValue());
            }
        }
        catch (tgt::Exception& e) {
            LERROR("Error while parsing VTK file: " << e.what());
            return;
        }
        catch (std::exception& e) {
            LERROR("Error while parsing VTK file: " << e.what());
            return;
        }

        applyInvalidationLevel(InvalidationLevel::VALID);
    }
}