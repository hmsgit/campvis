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
        : AbstractProcessor()
        , p_url("url", "Image URL", "")
        , p_size("Size", "Image Size", tgt::ivec3(1), tgt::ivec3(1), tgt::ivec3(2048))
        , p_numChannels("NumChannels", "Number of Channels per Element", 1, 1, 9)
        , p_baseType("BaseType", "Base Type", baseTypeOptions, 7)
        , p_offset("Offset", "Byte Offset", 0, 0, std::numeric_limits<int>::max())
        , p_endianness("Endianess", "Endianess", endianOptions, 2)
        , p_targetImageID("targetImageName", "Target Image ID", "RawImageReader.output", DataNameProperty::WRITE)
        , p_imageOffset("ImageOffset", "Image Offset in mm", tgt::vec3(0.f), tgt::vec3(-10000.f), tgt::vec3(10000.f), tgt::vec3(0.1f))
        , p_voxelSize("VoxelSize", "Voxel Size in mm", tgt::vec3(1.f), tgt::vec3(-100.f), tgt::vec3(100.f), tgt::vec3(0.1f))
    {
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
        p_targetImageID.issueWrite();

        validate(INVALID_RESULT);
    }
}
