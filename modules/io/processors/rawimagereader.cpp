// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "rawimagereader.h"

#include <fstream>
#include <limits>

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationdisk.h"

namespace campvis {
    static const GenericOption<WeaklyTypedPointer::BaseType> baseTypeOptions[7] = {
        GenericOption<WeaklyTypedPointer::BaseType>("uint8", "uint8", WeaklyTypedPointer::UINT8),
        GenericOption<WeaklyTypedPointer::BaseType>("int8", "int8", WeaklyTypedPointer::INT8),
        GenericOption<WeaklyTypedPointer::BaseType>("uint16", "uint16", WeaklyTypedPointer::UINT16),
        GenericOption<WeaklyTypedPointer::BaseType>("int16", "int16", WeaklyTypedPointer::INT16),
        GenericOption<WeaklyTypedPointer::BaseType>("uint32", "uint32", WeaklyTypedPointer::UINT32),
        GenericOption<WeaklyTypedPointer::BaseType>("int32", "int32", WeaklyTypedPointer::INT32),
        GenericOption<WeaklyTypedPointer::BaseType>("float", "float", WeaklyTypedPointer::FLOAT),
    };

    static const GenericOption<EndianHelper::Endianness> endianOptions[2] = {
        GenericOption<EndianHelper::Endianness>("b", "Big Endian", EndianHelper::IS_BIG_ENDIAN),
        GenericOption<EndianHelper::Endianness>("e", "Little Endian", EndianHelper::IS_LITTLE_ENDIAN),
    };
    
    const std::string RawImageReader::loggerCat_ = "CAMPVis.modules.io.RawImageReader";

    RawImageReader::RawImageReader() 
        : AbstractImageReader()
        , p_size("Size", "Image Size", tgt::ivec3(1), tgt::ivec3(1), tgt::ivec3(2048))
        , p_numChannels("NumChannels", "Number of Channels per Element", 1, 1, 9)
        , p_baseType("BaseType", "Base Type", baseTypeOptions, 7)
        , p_offset("Offset", "Byte Offset", 0, 0, std::numeric_limits<int>::max())
        , p_endianness("Endianess", "Endianess", endianOptions, 2)
        , p_imageOffset("ImageOffset", "Image Offset in mm", tgt::vec3(0.f), tgt::vec3(-10000.f), tgt::vec3(10000.f), tgt::vec3(0.1f))
        , p_voxelSize("VoxelSize", "Voxel Size in mm", tgt::vec3(1.f), tgt::vec3(-100.f), tgt::vec3(100.f), tgt::vec3(0.1f))
    {
		this->_ext = "raw";
		this->p_targetImageID.setValue("RawImageReader.output");
        
        addProperty(&p_url);
        addProperty(&p_size);
        addProperty(&p_numChannels);
        addProperty(&p_baseType);
        addProperty(&p_offset);
        addProperty(&p_endianness);
        addProperty(&p_targetImageID);
        addProperty(&p_imageOffset);
        addProperty(&p_voxelSize);
    }

    RawImageReader::~RawImageReader() {

    }

    void RawImageReader::process(DataContainer& data) {
        size_t dimensionality = 3;
        if (p_size.getValue().z == 1) {
            dimensionality = (p_size.getValue().y == 1) ? 1 : 2;
        }

        ImageData* image = new ImageData(dimensionality, p_size.getValue(), p_numChannels.getValue());
        ImageRepresentationDisk::create(image, p_url.getValue(), p_baseType.getOptionValue(), p_offset.getValue(), p_endianness.getOptionValue());
        image->setMappingInformation(ImageMappingInformation(p_size.getValue(), p_imageOffset.getValue(), p_voxelSize.getValue()));
        data.addData(p_targetImageID.getValue(), image);

        validate(INVALID_RESULT);
    }

	PropertyCollection& RawImageReader::getMetaProperties() {
		PropertyCollection dummy;
		return dummy;
	}
}
