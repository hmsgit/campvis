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
        tgt::mat4 t = tgt::mat4::createTranslation(_offset);
//	tgt::mat4 s = tgt::mat4::createScale(_voxelSize * _size);
//        _textureToWolrdTransformation = t * s;
        _textureToWolrdTransformation = tgt::mat4::createTranslation(_offset) * tgt::mat4::createScale(_voxelSize * _size);
//	std::cout << s << t << _textureToWolrdTransformation << "\n";
        if (! _textureToWolrdTransformation.invert(_worldToTextureTransformation))
            tgtAssert(false, "Could not invert texture-to-world matrix. That should not happen!");

	tgt::mat4 s = tgt::mat4::createScale(_voxelSize);
        _voxelToWorldTransformation = t * s; 
//	std::cout << s << t << _voxelToWorldTransformation << "\n";
        if (! _voxelToWorldTransformation.invert(_worldToVoxelTransformation))
            tgtAssert(false, "Could not invert voxel-to-world matrix. That should not happen!");
    }

    const tgt::mat4& ImageMappingInformation::getTextureToWorldMatrix() const {
        return _textureToWolrdTransformation;
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
