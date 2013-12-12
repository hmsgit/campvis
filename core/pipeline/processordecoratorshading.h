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

#ifndef PROCESSORDECORATORSHADING_H__
#define PROCESSORDECORATORSHADING_H__

#include "tgt/textureunit.h"
#include "core/pipeline/processordecoratorgradient.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/optionproperty.h"

namespace campvis {

    class ProcessorDecoratorShading : public ProcessorDecoratorGradient {
    public:
        explicit ProcessorDecoratorShading(const std::string& lightUniformName = "_lightSource");

        virtual ~ProcessorDecoratorShading();

    protected:
        void addProperties(HasPropertyCollection* propCollection);

        void renderProlog(const DataContainer& dataContainer, tgt::Shader* shader);

        std::string generateHeader() const;


        BoolProperty _enableShading;        ///< Flag whether to enable shading;
        Vec3Property _lightPosition;        ///< Light position
        Vec3Property _ambientColor;         ///< Ambient light color
        Vec3Property _diffuseColor;         ///< Diffuse light color
        Vec3Property _specularColor;        ///< Specular light color
        FloatProperty _shininess;           ///< Specular shininess
        Vec3Property _attenuation;          ///< Attenuation factors

        const std::string _lightUniformName;    ///< Uniform name for light
    };

}

#endif // PROCESSORDECORATORSHADING_H__
