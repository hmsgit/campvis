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

#include "processordecoratorgradient.h"

#include "tgt/shadermanager.h"
#include "core/properties/propertycollection.h"

namespace campvis {
    static const GenericOption<ProcessorDecoratorGradient::GradientMethod> gradientOptions[4] = {
        GenericOption<ProcessorDecoratorGradient::GradientMethod>("Forward", "Forward Differences", ProcessorDecoratorGradient::ForwardDifferences),
        GenericOption<ProcessorDecoratorGradient::GradientMethod>("Central", "Central Differences", ProcessorDecoratorGradient::CentralDifferences),
        GenericOption<ProcessorDecoratorGradient::GradientMethod>("Sobel", "Sobel Filter", ProcessorDecoratorGradient::SobelFilter),
        GenericOption<ProcessorDecoratorGradient::GradientMethod>("FilteredCentral", "Filtered Central Differences", ProcessorDecoratorGradient::FilteredCentralDifferences)
    };

    ProcessorDecoratorGradient::ProcessorDecoratorGradient()
        : AbstractProcessorDecorator()
        , _gradientMethod("GradientMethod", "Gradient Computation Method", gradientOptions, 4, AbstractProcessor::INVALID_SHADER | AbstractProcessor::INVALID_RESULT)
    {
        _gradientMethod.setValue(1);
    }

    ProcessorDecoratorGradient::~ProcessorDecoratorGradient() {
    }

    void ProcessorDecoratorGradient::addProperties(HasPropertyCollection* propCollection) {
        propCollection->addProperty(&_gradientMethod);
    }

    std::string ProcessorDecoratorGradient::generateHeader() const {
        std::string toReturn;
        switch (_gradientMethod.getOptionValue()) {
            case ForwardDifferences:
                toReturn.append("#define computeGradient(tex, texParams, texCoords) computeGradientForwardDifferences(tex, texParams, texCoords)\n");
                break;
            case CentralDifferences:
                toReturn.append("#define computeGradient(tex, texParams, texCoords) computeGradientCentralDifferences(tex, texParams, texCoords)\n");
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

}
