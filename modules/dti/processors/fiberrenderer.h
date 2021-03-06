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

#ifndef FIBERRENDERER_H__
#define FIBERRENDERER_H__

#include <string>

#include "core/pipeline/visualizationprocessor.h"
#include "core/pipeline/abstractprocessordecorator.h"
#include "core/properties/allproperties.h"

#include "modules/modulesapi.h"

namespace cgt {
    class Shader;
}

namespace campvis {
namespace dti {

    /**
     * Renders Fiber Data
     */
    class CAMPVIS_MODULES_API FiberRenderer : public VisualizationProcessor {
    public:
        enum RenderMode {
            STRIPES,
            TUBES
        };

        enum ColoringMode {
            TANGENT,
            VIEW_DEPENDENT
        };

        /**
         * Constructs a new FiberRenderer Processor
         **/
        explicit FiberRenderer(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~FiberRenderer();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "FiberRenderer"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Renders Fiber Data"; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_strainId;                ///< ID for input strain data
        DataNameProperty p_camera;                  ///< ID for camera input
        DataNameProperty p_renderTargetID;          ///< image ID for output image

        GenericOptionProperty<RenderMode> p_renderMode;
        GenericOptionProperty<ColoringMode> p_coloringMode;

        FloatProperty p_lineWidth;

        BoolProperty p_enableShading;               ///< Flag whether to enable shading
        DataNameProperty p_lightId;                 ///< Name/ID for the LightSource to use

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// \see AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer);
        /// \see    AbstractProcessor::updateShader
        virtual void updateShader();

        /**
         * Generates the GLSL header.
         */
        std::string generateGlslHeader() const;

        cgt::Shader* _shader;                           ///< Shader for EEP generation

        static const std::string loggerCat_;
    };

}
}

#endif // FIBERRENDERER_H__
