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

#include "abstracttransferfunction.h"

#include <tbb/tbb.h>
#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/texture.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagerepresentationlocal.h"

namespace campvis {
    
    const std::string AbstractTransferFunction::loggerCat_ = "CAMPVis.core.classification.AbstractTransferFunction";

    AbstractTransferFunction::AbstractTransferFunction(const tgt::svec3& size, const tgt::vec2& intensityDomain /*= tgt::vec2(0.f, 1.f)*/)
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
        delete _texture;
        _texture = 0;
    }

    void AbstractTransferFunction::bind(tgt::Shader* shader, const tgt::TextureUnit& texUnit, const std::string& transFuncUniform /*= "_transferFunction"*/, const std::string& transFuncParamsUniform /*= "_transferFunctionParameters"*/) {
        tgtAssert(shader != 0, "Shader must not be 0.");

        {
            tbb::mutex::scoped_lock lock(_localMutex);
            if (_texture == 0 || _dirtyTexture) {
                shader->deactivate();
                createTexture();
                shader->activate();
            }
        }

        texUnit.activate();
        _texture->bind();

        bool tmp = shader->getIgnoreUniformLocationError();
        shader->setIgnoreUniformLocationError(true);
        shader->setUniform(transFuncUniform, texUnit.getUnitNumber());
        switch (getDimensionality()) {
            case 1:
                shader->setUniform(transFuncParamsUniform + "._intensityDomain", tgt::vec2(_intensityDomain));
                break;
            case 2:
                shader->setUniform(transFuncParamsUniform + "._intensityDomainX", tgt::vec2(_intensityDomain));
                break;
            default:
                tgtAssert(false, "Unsupported TF dimensionality!");
                break;
        }
        
        shader->setIgnoreUniformLocationError(tmp);
    }
    
    void AbstractTransferFunction::setIntensityDomain(const tgt::vec2& newDomain) {
        tgtAssert(newDomain.x <= newDomain.y, "Intensity domain is not a valid interval.");
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            _intensityDomain = newDomain;
        }
        s_intensityDomainChanged();
        s_changed();
    }

    const tgt::vec2& AbstractTransferFunction::getIntensityDomain() const {
        return _intensityDomain;
    }

    const tgt::Texture* AbstractTransferFunction::getTexture() {
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            if (_texture == 0 || _dirtyTexture) {
                createTexture();
            }
        }
        return _texture;
    }
    
}