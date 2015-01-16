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

#include "simpletransferfunction.h"

#include "cgt/assert.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/texture.h"
#include "cgt/textureunit.h"

namespace campvis {

    const std::string SimpleTransferFunction::loggerCat_ = "CAMPVis.core.classification.SimpleTransferFunction";

    SimpleTransferFunction::SimpleTransferFunction(size_t size, const cgt::vec2& intensityDomain /*= cgt::vec2(0.f, 1.f)*/) 
        : AbstractTransferFunction(cgt::svec3(size, 1, 1), intensityDomain)
        , _leftColor(0, 0, 0, 255)
        , _rightColor(255)
    {
    }

    SimpleTransferFunction::~SimpleTransferFunction() {

    }

    SimpleTransferFunction* SimpleTransferFunction::clone() const {
        SimpleTransferFunction* toReturn = new SimpleTransferFunction(_size.x, _intensityDomain);
        toReturn->setLeftColor(_leftColor);
        toReturn->setRightColor(_rightColor);
        return toReturn;
    }

    size_t SimpleTransferFunction::getDimensionality() const {
        return 1;
    }

    void SimpleTransferFunction::createTexture() {
        delete _texture;

        GLenum dataType = GL_UNSIGNED_BYTE;
        _texture = new cgt::Texture(GL_TEXTURE_1D, _size, GL_RGBA8, cgt::Texture::LINEAR);
        _texture->setWrapping(cgt::Texture::CLAMP_TO_EDGE);

        cgt::col4* texels = new cgt::col4[_size.x];
        cgt::col4 diff = _rightColor - _leftColor;
        for (size_t i = 0; i < _size.x; ++i) {
            float multiplier = static_cast<float>(i) / (_size.x - 1);
            cgt::col4& texel = texels[i];
            for (size_t j = 0; j < 4; ++j) {
                texel[j] = static_cast<uint8_t>(_leftColor[j] + (static_cast<float>(diff[j]) * multiplier));
            }
        }

        _texture->uploadTexture(reinterpret_cast<GLubyte*>(texels), GL_RGBA, GL_UNSIGNED_BYTE);
        _dirtyTexture = false;
    }

    void SimpleTransferFunction::setLeftColor(const cgt::col4& color) {
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            _leftColor = color;
        }
        _dirtyTexture = true;
        s_changed.emitSignal();
    }

    void SimpleTransferFunction::setRightColor(const cgt::col4& color) {
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            _rightColor = color;
        }
        _dirtyTexture = true;
        s_changed.emitSignal();
    }

    const cgt::col4& SimpleTransferFunction::getLeftColor() const {
        return _leftColor;
    }

    const cgt::col4& SimpleTransferFunction::getRightColor() const {
        return _rightColor;
    }

    cgt::vec2 SimpleTransferFunction::getVisibilityDomain() const {
        return cgt::vec2(0.f, 1.f);
    }


}