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

#ifndef RENDERTARGETCOMPOSITOR_H__
#define RENDERTARGETCOMPOSITOR_H__

#include <string>

#include "core/pipeline/abstractprocessordecorator.h"
#include "core/pipeline/processordecoratorbackground.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/optionproperty.h"

namespace tgt {
    class Shader;
}

namespace campvis {
    class ImageData;

    /**
     * Performs the composition of a multiple render targets
     */
    class RenderTargetCompositor : public VisualizationProcessor, public HasProcessorDecorators {
    public:
        enum CompositingMode {
            CompositingModeFirst = 0,
            CompositingModeSecond = 1,
            CompositingModeAlpha = 2,
            CompositingModeDifference = 3,
            CompositingModeDepth = 4
        };

        /**
         * Constructs a new RenderTargetCompositor Processor
         **/
        RenderTargetCompositor(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~RenderTargetCompositor();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "RenderTargetCompositor"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Combines Normal DVR and Virtual Mirror DVR images."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        virtual void process(DataContainer& data);

        DataNameProperty p_firstImageId;             ///< image ID for the first image to combine
        DataNameProperty p_secondImageId;            ///< image ID for the second image to combine
        DataNameProperty p_targetImageId;            ///< image ID for combined output image
        GenericOptionProperty<CompositingMode> p_compositingMethod;
        FloatProperty p_alphaValue;

    protected:
        tgt::Shader* _shader;                           ///< Shader for composition

        static const std::string loggerCat_;
    };

}

#endif // RENDERTARGETCOMPOSITOR_H__
