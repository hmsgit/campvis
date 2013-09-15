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

#include "processordecoratorshading.h"

#include "tgt/shadermanager.h"
#include "core/properties/propertycollection.h"

namespace campvis {

    ProcessorDecoratorShading::ProcessorDecoratorShading(const std::string& lightUniformName /*= "_lightSource"*/)
        : AbstractProcessorDecorator()
        , _enableShading("EnableShading", "Enable Shading", true, AbstractProcessor::INVALID_SHADER | AbstractProcessor::INVALID_RESULT)
        , _centralDifferences("CentralDifferences", "Use Central instead of Forward Differences", false, AbstractProcessor::INVALID_SHADER | AbstractProcessor::INVALID_RESULT)
        , _lightPosition("LightPosition", "Light Position", tgt::vec3(-8.f), tgt::vec3(-500.f), tgt::vec3(500.f), tgt::vec3(0.1f))
        , _ambientColor("AmbientColor", "Ambient Light Color", tgt::vec3(0.5f), tgt::vec3(0.f), tgt::vec3(1.f), tgt::vec3(0.01f))
        , _diffuseColor("DiffuseColor", "Diffuse Light Color", tgt::vec3(0.75f), tgt::vec3(0.f), tgt::vec3(1.f), tgt::vec3(0.01f))
        , _specularColor("SpecularColor", "Specular Light Color", tgt::vec3(0.5f), tgt::vec3(0.f), tgt::vec3(1.f), tgt::vec3(0.01f))
        , _shininess("Shininess", "Specular Shininess", 4.f, 0.f, 64.f, 0.5f)
        , _attenuation("Attenuation", "Attenuation Factors", tgt::vec3(0.f), tgt::vec3(0.f), tgt::vec3(1.f), tgt::vec3(0.01f))
        , _lightUniformName(lightUniformName)
    {
    }

    ProcessorDecoratorShading::~ProcessorDecoratorShading() {
    }

    void ProcessorDecoratorShading::addProperties(HasPropertyCollection* propCollection) {
        propCollection->addProperty(&_enableShading);
        propCollection->addProperty(&_centralDifferences);
        propCollection->addProperty(&_lightPosition);
        propCollection->addProperty(&_ambientColor);
        propCollection->addProperty(&_diffuseColor);
        propCollection->addProperty(&_specularColor);
        propCollection->addProperty(&_shininess);
        propCollection->addProperty(&_attenuation);
    }

    void ProcessorDecoratorShading::renderProlog(const DataContainer& dataContainer, tgt::Shader* shader) {
        shader->setUniform(_lightUniformName + "._position", _lightPosition.getValue());
        shader->setUniform(_lightUniformName + "._ambientColor", _ambientColor.getValue());
        shader->setUniform(_lightUniformName + "._diffuseColor", _diffuseColor.getValue());
        shader->setUniform(_lightUniformName + "._specularColor", _specularColor.getValue());
        shader->setUniform(_lightUniformName + "._shininess", _shininess.getValue());
        shader->setUniform(_lightUniformName + "._attenuation", _attenuation.getValue());
    }


    std::string ProcessorDecoratorShading::generateHeader() const {
        std::string toReturn;
        if (_enableShading.getValue())
            toReturn.append("#define ENABLE_SHADING\n");
        if (_centralDifferences.getValue())
            toReturn.append("#define computeGradient(tex, texParams,texCoords) computeGradientFilteredCentralDifferences(tex, texParams, texCoords)\n");
        else
            toReturn.append("#define computeGradient(tex, texParams,texCoords) computeGradientForwardDifferences(tex, texParams, texCoords)\n");

        return toReturn;
    }

}
