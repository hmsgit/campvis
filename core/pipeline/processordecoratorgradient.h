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

#ifndef PROCESSORDECORATORGRADIENT_H__
#define PROCESSORDECORATORGRADIENT_H__

#include "sigslot/sigslot.h"
#include "cgt/textureunit.h"

#include "core/pipeline/abstractprocessordecorator.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/optionproperty.h"

namespace campvis {

    /**
     * Processor decorator to decorate for all gradient computation methods supported in glsl/tools/gradient.frag.
     * Holds a single option property to select the gradient computation method and implements
     * generateHeader() to define computeGradient(tex, texParams, texCoords) in GLSL calling
     * the selected function.
     */
    class CAMPVIS_CORE_API ProcessorDecoratorGradient : public AbstractProcessorDecorator, public sigslot::has_slots {
    public:
        /// Method for online-calculating gradients
        enum GradientMethod {
            ForwardDifferences,
            CentralDifferences,
            FilteredCentralDifferences,
            SobelFilter
        };

        /// Constructor
        ProcessorDecoratorGradient();
        /// Virtual Destructor
        virtual ~ProcessorDecoratorGradient();

    protected:
        /// \see AbstractProcessorDecorator::addProperties()
        void addProperties(AbstractProcessor* propCollection);
        /// \see AbstractProcessorDecorator::renderProlog()
        virtual void renderProlog(const DataContainer& dataContainer, cgt::Shader* shader);
        /// \see AbstractProcessorDecorator::generateHeader()
        std::string generateHeader() const;

        /// Callback method when p_gradientMethod has changed
        void onGradientMethodChanged(const AbstractProperty* prop);

        GenericOptionProperty<GradientMethod> p_gradientMethod;  ///< Method for calculating the gradients
        FloatProperty p_lod;        ///< LOD to use for texture lookup during gradient computation

    };

}

#endif // PROCESSORDECORATORGRADIENT_H__
