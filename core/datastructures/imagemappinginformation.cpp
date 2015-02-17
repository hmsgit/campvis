
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

#include "imagemappinginformation.h"

#include "cgt/assert.h"
#include "cgt/logmanager.h"

namespace campvis {

    ImageMappingInformation::ImageMappingInformation(const cgt::vec3& size, const cgt::vec3& offset, const cgt::vec3& voxelSize, const cgt::mat4& customTransformation /*= LinearMapping<float>::identity*/)
        : _size(size)
        , _offset(offset)
        , _voxelSize(voxelSize)
        , _customTransformation(customTransformation)
    {
        cgt::mat4 invTrafo;
        if (! _customTransformation.invert(invTrafo)) {
            LERRORC("CAMPVis.core.ImageMappingInformation", "Custom transformation is not invertable! Resetting to identity tranformation.");
            _customTransformation = cgt::mat4::identity;
        }            

        updateMatrices();
    }

    const cgt::vec3& ImageMappingInformation::getOffset() const {
        return _offset;
    }

    const cgt::vec3& ImageMappingInformation::getVoxelSize() const {
        return _voxelSize;
    }

    void ImageMappingInformation::updateMatrices() {
        _textureToWorldTransformation = _customTransformation * cgt::mat4::createTranslation(_offset) * cgt::mat4::createScale(_voxelSize * _size);
        if (! _textureToWorldTransformation.invert(_worldToTextureTransformation))
            cgtAssert(false, "Could not invert texture-to-world matrix. That should not happen!");

        _voxelToWorldTransformation = _customTransformation * cgt::mat4::createTranslation(_offset) * cgt::mat4::createScale(_voxelSize);
        if (! _voxelToWorldTransformation.invert(_worldToVoxelTransformation))
            cgtAssert(false, "Could not invert voxel-to-world matrix. That should not happen!");
    }

    const cgt::mat4& ImageMappingInformation::getTextureToWorldMatrix() const {
        return _textureToWorldTransformation;
    }

    const cgt::mat4& ImageMappingInformation::getWorldToTextureMatrix() const {
        return _worldToTextureTransformation;
    }

    const cgt::mat4& ImageMappingInformation::getVoxelToWorldMatrix() const {
        return _voxelToWorldTransformation;
    }

    const cgt::mat4& ImageMappingInformation::getWorldToVoxelMatrix() const {
        return _worldToVoxelTransformation;
    }

    bool ImageMappingInformation::operator==(const ImageMappingInformation& obj) const{
        return (this->_offset == obj._offset)
            && (this->_size == obj._size)
            && (this->_voxelSize == obj._voxelSize)
            && (this->_customTransformation == obj._customTransformation)
            && (this->_textureToWorldTransformation == obj._textureToWorldTransformation)
            && (this->_voxelToWorldTransformation == obj._voxelToWorldTransformation)
            && (this->_worldToTextureTransformation == obj._worldToTextureTransformation)
            && (this->_worldToVoxelTransformation == obj._worldToVoxelTransformation);
    }

    const cgt::mat4 ImageMappingInformation::getCustomTransformation() const {
        return _customTransformation;
    }

}
