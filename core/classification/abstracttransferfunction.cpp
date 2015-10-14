// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#include "abstracttransferfunction.h"

#include <tbb/tbb.h>
#include "cgt/assert.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/texture.h"
#include "cgt/textureunit.h"

#include "core/datastructures/imagerepresentationlocal.h"

namespace campvis {
    
    const std::string AbstractTransferFunction::loggerCat_ = "CAMPVis.core.classification.AbstractTransferFunction";

    AbstractTransferFunction::AbstractTransferFunction(const cgt::svec3& size, const cgt::vec2& intensityDomain /*= cgt::vec2(0.f, 1.f)*/)
        : _size(size)
        , _intensityDomain(intensityDomain)
        , _texture(0)
    {
        _dirtyTexture = false;
    }

    AbstractTransferFunction::~AbstractTransferFunction() {
        if (_texture != 0)
            LWARNING("Called AbstractTransferFunction dtor without proper deinitialization - you just wasted resources!");
    }

    void AbstractTransferFunction::deinit() {
        s_intensityDomainChanged.disconnect_all();
        s_changed.disconnect_all();

        delete _texture;
        _texture = 0;
    }

    void AbstractTransferFunction::bind(cgt::Shader* shader, const cgt::TextureUnit& texUnit, const std::string& transFuncUniform /*= "_transferFunction"*/, const std::string& transFuncParamsUniform /*= "_transferFunctionParameters"*/) {
        cgtAssert(shader != 0, "Shader must not be 0.");

        {
            tbb::mutex::scoped_lock lock(_localMutex);
            if (_texture == 0 || _dirtyTexture) {
                if (shader->isActivated()) {
                    shader->deactivate();
                    createTexture();
                    shader->activate();
                }
                else {
                    createTexture();
                }
            }
        }

        texUnit.activate();
        _texture->bind();

        bool tmp = shader->getIgnoreUniformLocationError();
        shader->setIgnoreUniformLocationError(true);
        shader->setUniform(transFuncUniform, texUnit.getUnitNumber());
        switch (getDimensionality()) {
            case 1:
                shader->setUniform(transFuncParamsUniform + "._intensityDomain", cgt::vec2(_intensityDomain));
                break;
            case 2:
                shader->setUniform(transFuncParamsUniform + "._intensityDomainX", cgt::vec2(_intensityDomain));
                break;
            default:
                cgtAssert(false, "Unsupported TF dimensionality!");
                break;
        }
        
        shader->setIgnoreUniformLocationError(tmp);
    }
    
    void AbstractTransferFunction::setIntensityDomain(const cgt::vec2& newDomain) {
        cgtAssert(newDomain.x <= newDomain.y, "Intensity domain is not a valid interval.");
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            _intensityDomain = newDomain;
        }
        s_intensityDomainChanged.emitSignal();
        s_changed.emitSignal();
    }

    const cgt::vec2& AbstractTransferFunction::getIntensityDomain() const {
        return _intensityDomain;
    }

    const cgt::Texture* AbstractTransferFunction::getTexture() {
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            if (_texture == 0 || _dirtyTexture) {
                createTexture();
            }
        }
        return _texture;
    }
    const cgt::svec3& AbstractTransferFunction::getSize() {
        return _size;
    }
}