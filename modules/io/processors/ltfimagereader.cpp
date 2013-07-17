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

#include "ltfimagereader.h"

#include <limits>

#include "tgt/filesystem.h"
#include "core/tools/stringutils.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imageseries.h"
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
    
    const std::string LtfImageReader::loggerCat_ = "CAMPVis.modules.io.LtfImageReader";

    LtfImageReader::LtfImageReader() 
        : AbstractProcessor()
        , p_url("url", "Image URL", "")
        , p_size("Size", "Image Size", tgt::ivec3(1), tgt::ivec3(1), tgt::ivec3(2048))
        , p_numChannels("NumChannels", "Number of Channels per Element", 1, 1, 9)
        , p_baseType("BaseType", "Base Type", baseTypeOptions, 7)
        , p_targetImageID("targetImageName", "Target Image ID", "LtfImageReader.output", DataNameProperty::WRITE)
        , p_imageOffset("ImageOffset", "Image Offset in mm", tgt::vec3(0.f), tgt::vec3(-10000.f), tgt::vec3(10000.f))
        , p_voxelSize("VoxelSize", "Voxel Size in mm", tgt::vec3(1.f), tgt::vec3(-100.f), tgt::vec3(100.f))
    {
        addProperty(&p_url);
        addProperty(&p_size);
        addProperty(&p_numChannels);
        addProperty(&p_baseType);
        addProperty(&p_targetImageID);
        addProperty(&p_imageOffset);
        addProperty(&p_voxelSize);
    }

    LtfImageReader::~LtfImageReader() {

    }

    void LtfImageReader::process(DataContainer& data) {
        size_t dimensionality = 3;
        if (p_size.getValue().z == 1) {
            dimensionality = (p_size.getValue().y == 1) ? 1 : 2;
        }

        std::string noExt = tgt::FileSystem::fullBaseName(p_url.getValue());
        std::string::size_type offset = noExt.find_last_not_of("0123456789");
        if (offset != std::string::npos) {
            ++offset;
            std::string base = noExt.substr(0, offset);
            int index = StringUtils::fromString<int>(noExt.substr(offset));
            ImageSeries* series = new ImageSeries();

            while (tgt::FileSystem::fileExists(base + StringUtils::toString(index, noExt.size() - offset, '0') + ".ltf")) {
                ImageData* image = new ImageData(dimensionality, p_size.getValue(), p_numChannels.getValue());
                ImageRepresentationDisk::create(image, base + StringUtils::toString(index, noExt.size() - offset, '0') + ".ltf", p_baseType.getOptionValue(), 0, EndianHelper::getLocalEndianness(), tgt::svec3::zero, true);
                image->setMappingInformation(ImageMappingInformation(p_size.getValue(), p_imageOffset.getValue(), p_voxelSize.getValue()));
                series->addImage(image);
                ++index;
            }

            data.addData(p_targetImageID.getValue(), series);
            p_targetImageID.issueWrite();
        }
        else {
            LERROR("Could not deduce counting index.");
        }

        validate(INVALID_RESULT);
    }
}