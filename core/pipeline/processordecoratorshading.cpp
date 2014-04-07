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

#include "processordecoratorshading.h"

#include "tgt/shadermanager.h"
#include "core/pipeline/abstractprocessor.h"

namespace campvis {
    ProcessorDecoratorShading::ProcessorDecoratorShading(const std::string& lightUniformName /*= "_lightSource"*/)
        : ProcessorDecoratorGradient()
        , _enableShading("EnableShading", "Enable Shading", true)
        , _lightPosition("LightPosition", "Light Position", tgt::vec3(-100.f), tgt::vec3(-500.f), tgt::vec3(500.f), tgt::vec3(1.f))
        , _ambientColor("AmbientColor", "Ambient Light Color", tgt::vec3(0.4f), tgt::vec3(0.f), tgt::vec3(1.f))
        , _diffuseColor("DiffuseColor", "Diffuse Light Color", tgt::vec3(0.75f), tgt::vec3(0.f), tgt::vec3(1.f))
        , _specularColor("SpecularColor", "Specular Light Color", tgt::vec3(0.6f), tgt::vec3(0.f), tgt::vec3(1.f))
        , _shininess("Shininess", "Specular Shininess", 24.f, 0.f, 64.f, 0.5f)
        , _attenuation("Attenuation", "Attenuation Factors", tgt::vec3(0.f), tgt::vec3(0.f), tgt::vec3(1.f))
        , _lightUniformName(lightUniformName)
    {
    }

    ProcessorDecoratorShading::~ProcessorDecoratorShading() {
    }

    void ProcessorDecoratorShading::addProperties(AbstractProcessor* propCollection) {
        propCollection->addProperty(_enableShading, AbstractProcessor::INVALID_SHADER | AbstractProcessor::INVALID_RESULT);

        ProcessorDecoratorGradient::addProperties(propCollection);
        propCollection->addProperty(_lightPosition);
        propCollection->addProperty(_ambientColor);
        propCollection->addProperty(_diffuseColor);
        propCollection->addProperty(_specularColor);
        propCollection->addProperty(_shininess);
        propCollection->addProperty(_attenuation);
    }

    void ProcessorDecoratorShading::renderProlog(const DataContainer& dataContainer, tgt::Shader* shader) {
        ProcessorDecoratorGradient::renderProlog(dataContainer, shader);

        shader->setUniform(_lightUniformName + "._position", _lightPosition.getValue());
        shader->setUniform(_lightUniformName + "._ambientColor", _ambientColor.getValue());
        shader->setUniform(_lightUniformName + "._diffuseColor", _diffuseColor.getValue());
        shader->setUniform(_lightUniformName + "._specularColor", _specularColor.getValue());
        shader->setUniform(_lightUniformName + "._shininess", _shininess.getValue());
        shader->setUniform(_lightUniformName + "._attenuation", _attenuation.getValue());
    }


    std::string ProcessorDecoratorShading::generateHeader() const {
        std::string toReturn = ProcessorDecoratorGradient::generateHeader();

        if (_enableShading.getValue())
            toReturn.append("#define ENABLE_SHADING\n");
        return toReturn;
    }

}
