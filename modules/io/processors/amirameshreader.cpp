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

#include "amirameshreader.h"

#include <fstream>

#include "cgt/filesystem.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationdisk.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/tools/textfileparser.h"

/*
 * ATTENTION: This reader has only a very limited support for AmiraMesh files, if
 * your file is not supported properly, you may have to extend this class.
 * 
 * Currently supported:
 *  - uniform rectlinear grids of basic data types
 * 
 * Some references:
 *  - http://people.mpi-inf.mpg.de/~weinkauf/notes/amiramesh.html
 */

namespace campvis {
    const std::string AmiraMeshReader::loggerCat_ = "CAMPVis.modules.io.AmiraMeshReader";

    AmiraMeshReader::AmiraMeshReader() 
        : AbstractImageReader()
        , p_imageOffset("ImageOffset", "Image Offset in mm", cgt::vec3(0.f), cgt::vec3(-10000.f), cgt::vec3(10000.f), cgt::vec3(0.1f))
        , p_voxelSize("VoxelSize", "Voxel Size in mm", cgt::vec3(1.f), cgt::vec3(-100.f), cgt::vec3(100.f), cgt::vec3(0.1f))
    {
        this->_ext.push_back("am");
        this->p_targetImageID.setValue("AmiraMeshReader.output");

        addProperty(p_url);
        addProperty(p_targetImageID);
        addProperty(p_imageOffset);
        addProperty(p_voxelSize);
    }

    AmiraMeshReader::~AmiraMeshReader() {

    }

namespace {
    std::string getTrimmedLine(std::ifstream& file, char delim = '\n') {
        std::string toReturn;
        std::getline(file, toReturn, delim);
        return StringUtils::trim(toReturn);
    }

    std::string advanceTo(std::ifstream& file, const std::string& startString) {
        while (file.good()) {
            std::string toReturn = getTrimmedLine(file);
            if (toReturn.compare(0, startString.size(), startString) == 0)
                return toReturn;
        }

        return "";
    }
}

    void AmiraMeshReader::updateResult(DataContainer& data) {
        try {
            std::ifstream file(p_url.getValue().c_str(), std::ifstream::in | std::ios::binary);
            if (!file.is_open() || file.bad())
                throw cgt::FileException("Could not open file " + p_url.getValue() + " for reading.", p_url.getValue());

            // identify as correct file type
            std::string curLine = getTrimmedLine(file);
            // cppcheck-suppress stlIfStrFind
            if (curLine.find("# AmiraMesh") != 0)
                throw cgt::FileException("Could not identify as AmiraMesh file.", p_url.getValue());
            // cppcheck-suppress stlIfStrFind
            if (curLine.find("# AmiraMesh BINARY-LITTLE-ENDIAN") != 0)
                throw cgt::FileException("Unsupported format in AmiraMesh file.", p_url.getValue());

            // get Lattice (image size)
            if ((curLine = advanceTo(file, "define Lattice ")).empty())
                throw cgt::FileException("Malformed (or unsupported?) AmiraMesh file.", p_url.getValue());

            std::vector<std::string> lattice = StringUtils::parseFloats(curLine.substr(15));
            if (lattice.size() != 3)
                throw cgt::FileException("Malformed (or unsupported?) AmiraMesh file.", p_url.getValue());
                
            cgt::svec3 size(StringUtils::fromString<size_t>(lattice[0]), StringUtils::fromString<size_t>(lattice[1]), StringUtils::fromString<size_t>(lattice[2]));
            cgt::vec3 imageOffset(0.f);
            cgt::vec3 voxelSize(1.f);
            cgt::mat4 transformationMatrix = cgt::mat4::identity;
            size_t numChannels = 1;
            WeaklyTypedPointer::BaseType baseType = WeaklyTypedPointer::UINT8;


            // get further parameters
            if ((curLine = advanceTo(file, "Parameters {")).empty())
                throw cgt::FileException("Malformed (or unsupported?) AmiraMesh file.", p_url.getValue());

            int nestedLevel = 0;
            do {
                curLine = getTrimmedLine(file);
                if (curLine.substr(curLine.length() - 1, 1) == "{") {
                    ++nestedLevel;
                }
                else if (curLine == "}") {
                    --nestedLevel;
                }
                else if (nestedLevel == 0) {
                    std::vector<std::string> parts = StringUtils::splitStringsafe(curLine, " \t", '"');
                    if (parts[0] == "CoordType") {
                        if (parts[1] != "uniform")
                            throw cgt::FileException("Unsupported CoordType in file.", p_url.getValue());
                    }
                    if (parts[0] == "TransformationMatrix") {
                        if (parts.size() != 17)
                            throw cgt::FileException("Unsupported TransformationMatrix structure in file.", p_url.getValue());
                        for (size_t i = 0; i < 16; ++i)
                            transformationMatrix.elem[i] = StringUtils::fromString<float>(parts[i+1]);
                        transformationMatrix = cgt::transpose(transformationMatrix);
                    }
                    if (parts[0] == "BoundingBox") {
                        if (parts.size() != 7)
                            throw cgt::FileException("Unsupported BoundingBox structure in file.", p_url.getValue());
                        cgt::vec3 llf, urb;
                        for (size_t i = 0; i < 3; ++i) {
                            llf[i] = StringUtils::fromString<float>(parts[2*i + 1]);
                            urb[i] = StringUtils::fromString<float>(parts[2*i + 2]);
                        }
                        imageOffset = llf;
                        voxelSize = (urb - llf) / cgt::vec3(size);
                    }
                }
            } while (file.good() && nestedLevel >= 0);


            // get data type
            if ((curLine = advanceTo(file, "Lattice { ")).empty())
                throw cgt::FileException("Malformed (or unsupported?) AmiraMesh file.", p_url.getValue());
            std::vector<std::string> parts = StringUtils::splitStringsafe(curLine, " \t");
            std::string dataType = parts[2];

            if (dataType == "unsigned") {
                dataType = parts[2] + " " + parts[3];
            }

            if (dataType[dataType.length() - 3] == '[' && dataType[dataType.length() - 1] == ']') {
                numChannels = StringUtils::fromString<size_t>(dataType.substr(dataType.length() - 2, 1));
                dataType = dataType.substr(0, dataType.length() - 3);
            }

            if (dataType == "unsigned char")
                baseType = WeaklyTypedPointer::UINT8;
            else if (dataType == "char")
                baseType = WeaklyTypedPointer::INT8;
            else if (dataType == "unsigned short")
                baseType = WeaklyTypedPointer::UINT16;
            else if (dataType == "short")
                baseType = WeaklyTypedPointer::INT16;
            else if (dataType == "unsigned int")
                baseType = WeaklyTypedPointer::UINT32;
            else if (dataType == "int")
                baseType = WeaklyTypedPointer::INT32;
            else if (dataType == "float")
                baseType = WeaklyTypedPointer::FLOAT;


            // compute offset
            if (advanceTo(file, "# Data section follows").empty())
                throw cgt::FileException("Malformed (or unsupported?) AmiraMesh file.", p_url.getValue());

            getTrimmedLine(file);
            size_t offset = file.tellg();



            // all parsing done - lets create the image:
            ImageData* image = new ImageData(3, size, numChannels);
            ImageRepresentationDisk::create(image, p_url.getValue(), baseType, offset, EndianHelper::IS_LITTLE_ENDIAN);
            image->setMappingInformation(ImageMappingInformation(size, imageOffset + p_imageOffset.getValue(), voxelSize * p_voxelSize.getValue(), transformationMatrix));
            data.addData(p_targetImageID.getValue(), image);
        }
        catch (cgt::Exception& e) {
            LERROR("Error while parsing AmiraMesh header: " << e.what());
            return;
        }
        catch (std::exception& e) {
            LERROR("Error while parsing AmiraMesh header: " << e.what());
            return;
        }
    }

}