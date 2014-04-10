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

#ifndef VOLUMEEXPLORER_H__
#define VOLUMEEXPLORER_H__

#include "tgt/event/eventlistener.h"

#include "core/eventhandlers/mwheeltonumericpropertyeventlistener.h"
#include "core/eventhandlers/trackballnavigationeventlistener.h"
#include "core/eventhandlers/transfuncwindowingeventlistener.h"

#include "core/pipeline/abstractprocessordecorator.h"
#include "core/pipeline/visualizationprocessor.h"

#include "core/properties/datanameproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/metaproperty.h"

#include "modules/vis/processors/volumerenderer.h"
#include "modules/vis/processors/sliceextractor.h"

namespace tgt {
    class Shader;
}

class FaceGeometry;

namespace campvis {
    /**
     * Combines a volume raycaster and 3 slice views for explorative volume visualization.
     */
    class VolumeExplorer : public VisualizationProcessor, public HasProcessorDecorators, public tgt::EventListener {
    public:
        /**
         * Constructs a new VolumeExplorer Processor
         * \param   viewportSizeProp    Pointer to the property defining the viewport size, must not be 0.
         * \param   raycaster           Raycaster to use for rendering, must not be 0, VolumeRenderer will take ownership.
         **/
        VolumeExplorer(IVec2Property* viewportSizeProp, RaycastingProcessor* raycaster = new SimpleRaycaster(0));

        /**
         * Destructor
         **/
        virtual ~VolumeExplorer();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "VolumeExplorer"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Combines a volume raycaster and 3 slice views for explorative volume visualization."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };
        
        /// \see tgt::EventListener::onEvent()
        virtual void onEvent(tgt::Event* e);

        DataNameProperty p_inputVolume;     ///< image ID for first input image
        DataNameProperty p_outputImage;     ///< image ID for output image

        BoolProperty p_enableScribbling;    ///< Enable Scribbling in Slice Views

        MetaProperty p_seProperties;        ///< MetaProperty for SliceExtractor properties
        MetaProperty p_vrProperties;        ///< MetaProperty for Raycaster properties


    protected:
        /// Additional invalidation levels for this processor.
        /// Not the most beautiful design though.
        enum ProcessorInvalidationLevel {
            VR_INVALID = FIRST_FREE_TO_USE_INVALIDATION_LEVEL,
            SLICES_INVALID = FIRST_FREE_TO_USE_INVALIDATION_LEVEL << 1,
            SCRIBBLE_INVALID = FIRST_FREE_TO_USE_INVALIDATION_LEVEL << 2,
        };

        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);

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

        /**
         * Callback called from SliceExtractor when a scribble has been painted.
         * \param   voxel   Voxel position of scribble
         */
        void onSliceExtractorScribblePainted(tgt::vec3 voxel);

        void composeFinalRendering(DataContainer& data);

        /// \see    AbstractProcessor::updateProperties
        void updateProperties(DataContainer& dc);

        tgt::Shader* _shader;                           ///< Shader for slice rendering
        FaceGeometry* _quad;

        VolumeRenderer _raycaster;
        SliceExtractor _sliceExtractor;

        IVec2Property p_sliceRenderSize;
        IVec2Property p_volumeRenderSize;


        MWheelToNumericPropertyEventListener _xSliceHandler;
        MWheelToNumericPropertyEventListener _ySliceHandler;
        MWheelToNumericPropertyEventListener _zSliceHandler;
        TransFuncWindowingEventListener _windowingHandler;
        TrackballNavigationEventListener* _trackballEH;
        bool _mousePressedInRaycaster;                  ///< Flag whether mouse was pressed in raycaster

        std::vector<tgt::vec3>* _scribblePointer;       ///< Pointer encoding whether the mouse was pressed (!= nullptr) and whether we have yes-scribbles or no-scribbles.
        std::vector<tgt::vec3> _yesScribbles;           ///< All voxels of the current yes-scribbles
        std::vector<tgt::vec3> _noScribbles;            ///< All voxels of the current no-scribbles

        static const std::string loggerCat_;
    };

}

#endif // VOLUMEEXPLORER_H__
