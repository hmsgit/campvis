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

#ifndef STRAINFIBERRENDERER_H__
#define STRAINFIBERRENDERER_H__


#include <string>

#include "core/pipeline/visualizationprocessor.h"
#include "core/pipeline/abstractprocessordecorator.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/optionproperty.h"
#include "core/properties/cameraproperty.h"

namespace tgt {
    class Shader;
}

namespace campvis {
    /**
     * Renders Strain Fibers
     */
    class StrainFiberRenderer : public VisualizationProcessor, public HasProcessorDecorators {
    public:
        enum RenderMode {
            STRIPES,
            TUBES
        };

        /**
         * Constructs a new StrainFiberRenderer Processor
         **/
        StrainFiberRenderer(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~StrainFiberRenderer();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "StrainFiberRenderer"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Renders Strain Fibers"; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_strainId;        ///< ID for input strain data
        DataNameProperty p_renderTargetID;   ///< image ID for output image
        CameraProperty p_camera;

        GenericOptionProperty<RenderMode> p_renderMode;
        FloatProperty p_lineWidth;
        Vec4Property p_color;                ///< rendering color

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// \see    AbstractProcessor::updateShader
        virtual void updateShader();

        /**
         * Generates the GLSL header.
         */
        std::string generateGlslHeader() const;

        tgt::Shader* _shader;                           ///< Shader for EEP generation

        static const std::string loggerCat_;
    };

}

#endif // STRAINFIBERRENDERER_H__
