// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
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