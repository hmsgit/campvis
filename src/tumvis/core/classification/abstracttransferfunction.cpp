#include "abstracttransferfunction.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/texture.h"
#include "tgt/textureunit.h"

namespace TUMVis {

    const std::string AbstractTransferFunction::loggerCat_ = "TUMVis.core.classification.AbstractTransferFunction";

    AbstractTransferFunction::AbstractTransferFunction(const tgt::svec3& size, const tgt::vec2& intensityDomain /*= tgt::vec2(0.f, 1.f)*/)
        : _size(size)
        , _intensityDomain(intensityDomain)
        , _texture(0)
    {
        _dirty = false;
    }

    AbstractTransferFunction::~AbstractTransferFunction() {
        if (_texture != 0)
            LWARNING("Called AbstractTransferFunction dtor without proper deinitialization - you just wasted resources!");
    }

    void AbstractTransferFunction::deinit() {
        delete _texture;
        _texture = 0;
    }

    void AbstractTransferFunction::bind(tgt::Shader* shader, const tgt::TextureUnit& texUnit, const std::string& textureUniform /*= "_tfTex"*/, const std::string& textureParametersUniform /*= "_tgTextureParameters"*/) {
        // TODO:    lock here or in createTexture?
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            if (_texture == 0 || _dirty) {
                createTexture();
            }
        }

        texUnit.activate();
        _texture->bind();

        bool tmp = shader->getIgnoreUniformLocationError();
        shader->setIgnoreUniformLocationError(true);
        // TODO:    set domain mapping uniforms
        shader->setUniform(textureUniform, texUnit.getUnitNumber());
        shader->setUniform(textureParametersUniform + "._intensityDomain", tgt::vec2(_intensityDomain));
        shader->setIgnoreUniformLocationError(tmp);
    }

    void AbstractTransferFunction::setIntensityDomain(const tgt::vec2& newDomain) {
        tgtAssert(newDomain.x <= newDomain.y, "Intensity domain is not a valid interval.");
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            _intensityDomain = newDomain;
        }
        s_changed();
    }

    const tgt::vec2& AbstractTransferFunction::getIntensityDomain() const {
        return _intensityDomain;
    }


}