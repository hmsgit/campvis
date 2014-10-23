// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

#include "processordecoratorgradient.h"

#include "cgt/shadermanager.h"
#include "core/pipeline/abstractprocessor.h"

namespace campvis {
    static const GenericOption<ProcessorDecoratorGradient::GradientMethod> gradientOptions[4] = {
        GenericOption<ProcessorDecoratorGradient::GradientMethod>("Forward", "Forward Differences", ProcessorDecoratorGradient::ForwardDifferences),
        GenericOption<ProcessorDecoratorGradient::GradientMethod>("Central", "Central Differences", ProcessorDecoratorGradient::CentralDifferences),
        GenericOption<ProcessorDecoratorGradient::GradientMethod>("Sobel", "Sobel Filter", ProcessorDecoratorGradient::SobelFilter),
        GenericOption<ProcessorDecoratorGradient::GradientMethod>("FilteredCentral", "Filtered Central Differences", ProcessorDecoratorGradient::FilteredCentralDifferences)
    };

    ProcessorDecoratorGradient::ProcessorDecoratorGradient()
        : AbstractProcessorDecorator()
        , p_gradientMethod("GradientMethod", "Gradient Computation Method", gradientOptions, 4)
        , p_lod("GradientLod", "LOD for Gradient Computation", 0.5f, 0.f, 5.f, .1f, 1)
    {
        p_gradientMethod.setValue(1);
        p_gradientMethod.s_changed.connect(this, &ProcessorDecoratorGradient::onGradientMethodChanged);
    }

    ProcessorDecoratorGradient::~ProcessorDecoratorGradient() {
        p_gradientMethod.s_changed.disconnect(this);
    }

    void ProcessorDecoratorGradient::addProperties(AbstractProcessor* propCollection) {
        propCollection->addProperty(p_gradientMethod, AbstractProcessor::INVALID_SHADER | AbstractProcessor::INVALID_RESULT);
        propCollection->addProperty(p_lod);
    }

    std::string ProcessorDecoratorGradient::generateHeader() const {
        std::string toReturn;
        switch (p_gradientMethod.getOptionValue()) {
            case ForwardDifferences:
                toReturn.append("#define computeGradient(tex, texParams, texCoords) computeGradientForwardDifferencesLod(tex, texParams, texCoords, _gradientLod)\n");
                toReturn.append("uniform float _gradientLod = 0.0;\n");
                break;
            case CentralDifferences:
                toReturn.append("#define computeGradient(tex, texParams, texCoords) computeGradientCentralDifferencesLod(tex, texParams, texCoords, _gradientLod)\n");
                toReturn.append("uniform float _gradientLod = 0.0;\n");
                break;
            case FilteredCentralDifferences:
                toReturn.append("#define computeGradient(tex, texParams, texCoords) computeGradientFilteredCentralDifferences(tex, texParams, texCoords)\n");
                break;
            case SobelFilter:
                toReturn.append("#define computeGradient(tex, texParams, texCoords) computeGradientSobel(tex, texCoords)\n");
                break;
            default:
                tgtAssert(false, "Invalid enum!");
                break;
        }

        return toReturn;
    }

    void ProcessorDecoratorGradient::renderProlog(const DataContainer& dataContainer, tgt::Shader* shader) {
        if (p_gradientMethod.getOptionValue() == ForwardDifferences || p_gradientMethod.getOptionValue() == CentralDifferences) {
            shader->setUniform("_gradientLod", p_lod.getValue());
        }
    }

    void ProcessorDecoratorGradient::onGradientMethodChanged(const AbstractProperty* prop) {
        p_lod.setVisible(p_gradientMethod.getOptionValue() == ForwardDifferences || p_gradientMethod.getOptionValue() == CentralDifferences);
    }

}
