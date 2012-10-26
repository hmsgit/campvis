// ================================================================================================
// 
// This file is part of the CAMPVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "simpletransferfunction.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/texture.h"
#include "tgt/textureunit.h"

namespace campvis {

    const std::string SimpleTransferFunction::loggerCat_ = "CAMPVis.core.classification.SimpleTransferFunction";

    SimpleTransferFunction::SimpleTransferFunction(size_t size, const tgt::vec2& intensityDomain /*= tgt::vec2(0.f, 1.f)*/) 
        : AbstractTransferFunction(tgt::svec3(size, 1, 1), intensityDomain)
        , _leftColor(0, 0, 0, 0)
        , _rightColor(255)
    {
    }

    SimpleTransferFunction::~SimpleTransferFunction() {

    }

    size_t SimpleTransferFunction::getDimensionality() const {
        return 1;
    }

    void SimpleTransferFunction::createTexture() {
        delete _texture;

        GLenum dataType = GL_UNSIGNED_BYTE;
        _texture = new tgt::Texture(_size, GL_RGBA, dataType, tgt::Texture::LINEAR);
        _texture->setWrapping(tgt::Texture::CLAMP);

        tgt::col4 diff = _rightColor - _leftColor;
        for (size_t i = 0; i < _size.x; ++i) {
            float multiplier = static_cast<float>(i) / _size.x;
            tgt::col4& texel = _texture->texel<tgt::col4>(i);
            for (size_t j = 0; j < 4; ++j) {
                texel[j] = static_cast<uint8_t>(_leftColor[j] + (static_cast<float>(diff[j]) * multiplier));
            }
        }

        _texture->uploadTexture();
        _dirtyTexture = false;
    }

    void SimpleTransferFunction::setLeftColor(const tgt::col4& color) {
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            _leftColor = color;
        }
        _dirtyTexture = true;
        s_changed();
    }

    void SimpleTransferFunction::setRightColor(const tgt::col4& color) {
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            _rightColor = color;
        }
        _dirtyTexture = true;
        s_changed();
    }

    const tgt::col4& SimpleTransferFunction::getLeftColor() const {
        return _leftColor;
    }

    const tgt::col4& SimpleTransferFunction::getRightColor() const {
        return _rightColor;
    }

}