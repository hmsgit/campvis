#include "simpletransferfunction.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/texture.h"
#include "tgt/textureunit.h"

namespace TUMVis {

    const std::string SimpleTransferFunction::loggerCat_ = "TUMVis.core.classification.SimpleTransferFunction";

    SimpleTransferFunction::SimpleTransferFunction(size_t size, const tgt::vec2& intensityDomain /*= tgt::vec2(0.f, 1.f)*/) 
        : AbstractTransferFunction(tgt::svec3(size, 1, 1), intensityDomain)
        , _leftColor(0, 0, 0, 255)
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
        _dirty = false;
    }

    void SimpleTransferFunction::setLeftColor(const tgt::col4& color) {
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            _leftColor = color;
        }
        s_Changed;
    }

    void SimpleTransferFunction::setRightColor(const tgt::col4& color) {
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            _leftColor = color;
        }
        s_Changed;
    }

}