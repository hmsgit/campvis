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

#include "imagemappinginformation.h"

#include "tgt/assert.h"

namespace campvis {

    ImageMappingInformation::ImageMappingInformation(const tgt::vec3& size, const tgt::vec3& offset, const tgt::vec3& voxelSize, const LinearMapping<float>& realWorldValueMapping /*= LinearMapping<float>::identity*/)
        : _size(size)
        , _offset(offset)
        , _voxelSize(voxelSize)
        , _realWorldValueMapping(realWorldValueMapping)
    {
        updateMatrices();
    }

    const tgt::vec3& ImageMappingInformation::getOffset() const {
        return _offset;
    }

    const tgt::vec3& ImageMappingInformation::getVoxelSize() const {
        return _voxelSize;
    }

    const LinearMapping<float>& ImageMappingInformation::getRealWorldMapping() const {
        return _realWorldValueMapping;
    }

    void ImageMappingInformation::setRealWorldMapping(const LinearMapping<float>& rwvm) {
        _realWorldValueMapping = rwvm;
    }

    void ImageMappingInformation::updateMatrices() {
        _textureToWorldTransformation = tgt::mat4::createTranslation(_offset) * tgt::mat4::createScale(_voxelSize * _size);
        if (! _textureToWorldTransformation.invert(_worldToTextureTransformation))
            tgtAssert(false, "Could not invert texture-to-world matrix. That should not happen!");

        _voxelToWorldTransformation = tgt::mat4::createTranslation(_offset) * tgt::mat4::createScale(_voxelSize);
        if (! _voxelToWorldTransformation.invert(_worldToVoxelTransformation))
            tgtAssert(false, "Could not invert voxel-to-world matrix. That should not happen!");
    }

    const tgt::mat4& ImageMappingInformation::getTextureToWorldMatrix() const {
        return _textureToWorldTransformation;
    }

    const tgt::mat4& ImageMappingInformation::getWorldToTextureMatrix() const {
        return _worldToTextureTransformation;
    }

    const tgt::mat4& ImageMappingInformation::getVoxelToWorldMatrix() const {
        return _voxelToWorldTransformation;
    }

    const tgt::mat4& ImageMappingInformation::getWorldToVoxelMatrix() const {
        return _worldToVoxelTransformation;
    }

}
