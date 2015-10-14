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

#ifndef VOLUMERENDERER_H__
#define VOLUMERENDERER_H__

#include "sigslot/sigslot.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/pipeline/raycastingprocessor.h"
#include "core/properties/allproperties.h"

#include "modules/modulesapi.h"
#include "modules/vis/processors/eepgenerator.h"
#include "modules/vis/processors/proxygeometrygenerator.h"
#include "modules/vis/processors/orientationoverlay.h"
#include "modules/vis/processors/simpleraycaster.h"

namespace cgt {
    class Shader;
}

namespace campvis {
    /**
     * Combines proxy geometry generator, entry/exit point generator and raycasting into one
     * easy-to-use volume renderer.
     */
    class CAMPVIS_MODULES_API VolumeRenderer : public VisualizationProcessor {
    public:
        /// Additional invalidation levels for this processor.
        /// Not the most beautiful design though.
        enum ProcessorInvalidationLevel {
            PG_INVALID = AbstractProcessor::FIRST_FREE_TO_USE_INVALIDATION_LEVEL << 1,
            EEP_INVALID = AbstractProcessor::FIRST_FREE_TO_USE_INVALIDATION_LEVEL << 2,
            RAYCASTER_INVALID = AbstractProcessor::FIRST_FREE_TO_USE_INVALIDATION_LEVEL << 3
        };

        /**
         * Constructs a new VolumeRenderer Processor
         * \param   viewportSizeProp    Pointer to the property defining the viewport size, must not be 0.
         * \param   raycaster           Raycaster to use for rendering, must not be 0, VolumeRenderer will take ownership.
         */
        VolumeRenderer(IVec2Property* viewportSizeProp, RaycastingProcessor* raycaster = new SimpleRaycaster(0));

        /**
         * Destructor
         **/
        virtual ~VolumeRenderer();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();
        
        /** 
         * To be used in ProcessorFactory static methods
         */
        static const std::string getId() { return "VolumeRenderer"; };
        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return getId(); };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Combines proxy geometry generator, entry/exit point generator and raycasting into one easy-to-use volume renderer."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };

        /// \see VisualizationPipeline::setViewportSizeProperty()
        virtual void setViewportSizeProperty(IVec2Property* viewportSizeProp);

        /**
         * Returns the used RaycastingProcessor.
         * \return  _raycaster
         */
        RaycastingProcessor* getRaycastingProcessor();

        DataNameProperty p_inputVolume;             ///< image ID for first input image
        DataNameProperty p_camera;                  ///< Camera ID
        DataNameProperty p_outputImage;             ///< image ID for output image

        BoolProperty p_profileRaycaster;            ///< Flag whether the raycaster's execution time shall be profiled
        GLuint _timerQueryRaycaster;                ///< OpenGL timer query for raycaster

        MetaProperty p_pgProps;                     ///< MetaProperty for properties of the ProxyGeometryGenerator processor
        MetaProperty p_eepProps;                    ///< MetaProperty for properties of the EEPGenerator processor
        MetaProperty p_raycasterProps;              ///< MetaProperty for properties of the raycasting processor
        MetaProperty p_orientationOverlayProps;     ///< MetaProperty for properties of the OrientationOverlay processor

        GenericOptionProperty<std::string> p_raycastingProcSelector;   ///< Name of the RaycastingProcessor to be used

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// \see    AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer);

        /**
         * Slot getting called when one of the observed processors got invalidated.
         * Invalidates this meta-processor with the corresponding level.
         * \param   processor   The processor that emitted the signal
         */
        virtual void onProcessorInvalidated(AbstractProcessor* processor);

        /**
         * \see VisualizationProcessor::onPropertyChanged
         */
        virtual void onPropertyChanged(const AbstractProperty* prop);

        ProxyGeometryGenerator _pgGenerator;
        EEPGenerator _eepGenerator;
        RaycastingProcessor* _raycaster;

        OrientationOverlay _orientationOverlay;

        static const std::string loggerCat_;
    };
}

#endif // VOLUMERENDERER_H__
