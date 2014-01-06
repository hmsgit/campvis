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

#ifndef GLGRADIENTVOLUMEGENERATOR_H__
#define GLGRADIENTVOLUMEGENERATOR_H__

#include <string>

#include "core/pipeline/abstractprocessordecorator.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"

namespace tgt {
    class Shader;
}

namespace campvis {
    /**
     * Creates the gradient volume for the given intensity volume using OpenGL.
     */
    class GlGradientVolumeGenerator : public VisualizationProcessor, public HasProcessorDecorators {
    public:
        /**
         * Constructs a new GlGradientVolumeGenerator Processor
         **/
        GlGradientVolumeGenerator(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~GlGradientVolumeGenerator();

        /// \see AbstractProcessor::init
        virtual void init();
        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "GlGradientVolumeGenerator"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Creates the gradient volume for the given intensity volume using OpenGL."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        /// \see AbstractProcessor::process()
        virtual void process(DataContainer& data);

        DataNameProperty p_inputImage;      ///< ID for input volume
        DataNameProperty p_outputImage;     ///< ID for output gradient volume

    protected:
        std::string generateHeader() const;

        tgt::Shader* _shader;       ///< Shader for gradient volume generation

        static const std::string loggerCat_;
    };

}

#endif // GLGRADIENTVOLUMEGENERATOR_H__