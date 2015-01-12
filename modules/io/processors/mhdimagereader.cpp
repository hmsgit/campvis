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

#include "mhdimagereader.h"

#include <fstream>

#include "cgt/filesystem.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationdisk.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/tools/textfileparser.h"

/*
 * Full format specification at http://www.itk.org/Wiki/MetaIO/Documentation
 */

namespace campvis {
    const std::string MhdImageReader::loggerCat_ = "CAMPVis.modules.io.MhdImageReader";

    MhdImageReader::MhdImageReader() 
        : AbstractImageReader()
        , p_imageOffset("ImageOffset", "Image Offset in mm", cgt::vec3(0.f), cgt::vec3(-10000.f), cgt::vec3(10000.f), cgt::vec3(0.1f))
        , p_voxelSize("VoxelSize", "Voxel Size in mm", cgt::vec3(1.f), cgt::vec3(-100.f), cgt::vec3(100.f), cgt::vec3(0.1f))
    {
        this->_ext.push_back("mhd");
        this->p_targetImageID.setValue("MhdImageReader.output");
        addProperty(p_url);
        addProperty(p_targetImageID);
        addProperty(p_imageOffset);
        addProperty(p_voxelSize);
    }

    MhdImageReader::~MhdImageReader() {

    }

    void MhdImageReader::updateResult(DataContainer& data) {
        try {
            // start parsing
            TextFileParser tfp(p_url.getValue(), true, "=");
            tfp.parse<TextFileParser::ItemSeparatorLines>();

            // init optional parameters with sane default values
            std::string url;
            size_t dimensionality;
            cgt::svec3 size;
            WeaklyTypedPointer::BaseType pt;
            size_t numChannels = 1;
            size_t offset = 0;
            EndianHelper::Endianness e = EndianHelper::IS_LITTLE_ENDIAN;

            cgt::vec3 voxelSize(1.f);
            cgt::vec3 imageOffset(0.f);

            // image type
            if (tfp.hasKey("ObjectType")) {
                if (tfp.getString("ObjectType") == "Image") {
                    numChannels = 1;
                }
                else if (tfp.getString("ObjectType") == "TensorImage") {
                    numChannels = 6;
                }
                else {
                    LERROR("Error while parsing MHD header: ObjectType = Image or ObjectType = TensorImage expected");
                    return;
                }
            }
            else {
                LWARNING("No Key 'ObjectType' found - assuming Image.");
            }

            // dimensionality and size
            dimensionality = tfp.getSizeT("NDims");
            if (dimensionality == 2)
                size = cgt::svec3(tfp.getSvec2("DimSize"), 1);
            else if (dimensionality == 3)
                size = tfp.getSvec3("DimSize");
            else {
                LERROR("Error while parsing MHD header: Unsupported dimensionality: " << dimensionality);
                return;
            }

            // element type
            std::string et = tfp.getString("ElementType");
            if (et == "MET_UCHAR")
                pt = WeaklyTypedPointer::UINT8;
            else if (et == "MET_CHAR")
                pt = WeaklyTypedPointer::INT8;
            else if (et == "MET_USHORT")
                pt = WeaklyTypedPointer::UINT16;
            else if (et == "MET_SHORT")
                pt = WeaklyTypedPointer::INT16;
            else if (et == "MET_UINT")
                pt = WeaklyTypedPointer::UINT32;
            else if (et == "MET_INT")
                pt = WeaklyTypedPointer::INT32;
            else if (et == "MET_FLOAT")
                pt = WeaklyTypedPointer::FLOAT;
            else {
                LERROR("Error while parsing MHD header: Unsupported element type: " << et);
                return;
            }

            // further optional parameters:
            if (tfp.hasKey("HeaderSize")) {
                // header size can be -1...
                int tmp = tfp.getInt("HeaderSize");
                if (tmp >= 0)
                    offset = static_cast<int>(tmp);
            }
            if (tfp.hasKey("ElementByteOrderMSB"))
                e = (tfp.getBool("ElementByteOrderMSB") ? EndianHelper::IS_BIG_ENDIAN : EndianHelper::IS_LITTLE_ENDIAN);
            
            if (tfp.hasKey("ElementSpacing")) {
                if (dimensionality == 3)
                    voxelSize = tfp.getVec3("ElementSpacing");
                else if (dimensionality == 2)
                    voxelSize = cgt::vec3(tfp.getVec2("ElementSpacing"), 1.f);
            }
            if (tfp.hasKey("Position")) {
                if (dimensionality == 3)
                    imageOffset = tfp.getVec3("Position");
                else if (dimensionality == 2)
                    imageOffset = cgt::vec3(tfp.getVec2("Position"), 0.f);
            }
            if (tfp.hasKey("Offset")) {
                if (dimensionality == 3)
                    imageOffset = tfp.getVec3("Offset");
                else if (dimensionality == 2)
                    imageOffset = cgt::vec3(tfp.getVec2("Offset"), 0.f);
            }
            if (tfp.hasKey("VolumePosition")) {
                if (dimensionality == 3)
                    imageOffset = tfp.getVec3("VolumePosition");
                else if (dimensionality == 2)
                    imageOffset = cgt::vec3(tfp.getVec2("VolumePosition"), 0.f);
            }
            if (tfp.hasKey("ElementNumberOfChannels")) {
                numChannels = tfp.getSizeT("ElementNumberOfChannels");
            }

            // get raw image location:
            url = StringUtils::trim(tfp.getString("ElementDataFile"));
            if (url == "LOCAL") {
                url = p_url.getValue();
                // find beginning of local data:
                cgt::File* file = FileSys.open(p_url.getValue());
                if (!file || !file->isOpen())
                    throw cgt::FileException("Could not open file " + p_url.getValue() + " for reading.", p_url.getValue());

                while (!file->eof()) {
                    std::string line = StringUtils::trim(file->getLine());
                    if (line.find("ElementDataFile") == 0) {
                        offset = file->tell();
                    }
                file->close();
                delete file;
                }
            }
            else if (url == "LIST") {
                LERROR("Error while loading MHD file: Image list currently not supported.");
                return;
            }
            else {
                url = cgt::FileSystem::cleanupPath(cgt::FileSystem::dirName(p_url.getValue()) + "/" + url);
            } 



            // all parsing done - lets create the image:
            ImageData* image = new ImageData(dimensionality, size, numChannels);
            ImageRepresentationDisk::create(image, url, pt, offset, e);
            image->setMappingInformation(ImageMappingInformation(size, imageOffset + p_imageOffset.getValue(), voxelSize * p_voxelSize.getValue()));
            data.addData(p_targetImageID.getValue(), image);
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