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

#include "lightsourcedata.h"
#include "tgt/shadermanager.h"

namespace campvis {
    
    LightSourceData::LightSourceData(const tgt::vec3& lightPosition, const tgt::vec3& ambientColor /*= tgt::vec3(.4f)*/, const tgt::vec3& diffuseColor /*= tgt::vec3(.75f)*/, const tgt::vec3& specularColor /*= tgt::vec3(.6f)*/, float shininess /*= 24.f*/, const tgt::vec3& attenuation /*= tgt::vec3(0.f)*/)
        : AbstractData()
        , _lightPosition(lightPosition)
        , _ambientColor(ambientColor)
        , _diffuseColor(diffuseColor)
        , _specularColor(specularColor)
        , _shininess(shininess)
        , _attenuation(attenuation)
    {

    }

    LightSourceData::~LightSourceData() {

    }

    LightSourceData* LightSourceData::clone() const {
        return new LightSourceData(*this);
    }

    size_t LightSourceData::getLocalMemoryFootprint() const {
        return sizeof(LightSourceData);
    }

    size_t LightSourceData::getVideoMemoryFootprint() const {
        return 0;
    }

    void LightSourceData::bind(tgt::Shader* shader, const std::string& uniformName) const {
        tgt::Shader::IgnoreUniformLocationErrorGuard guard(shader);

        shader->setUniform(uniformName + "._position", _lightPosition);
        shader->setUniform(uniformName + "._ambientColor", _ambientColor);
        shader->setUniform(uniformName + "._diffuseColor", _diffuseColor);
        shader->setUniform(uniformName + "._specularColor", _specularColor);
        shader->setUniform(uniformName + "._shininess", _shininess);
        shader->setUniform(uniformName + "._attenuation", _attenuation);
    }

}