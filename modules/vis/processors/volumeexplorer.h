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

#ifndef VOLUMEEXPLORER_H__
#define VOLUMEEXPLORER_H__

#include "cgt/event/eventlistener.h"

#include "core/eventhandlers/mwheeltonumericpropertyeventlistener.h"
#include "core/eventhandlers/transfuncwindowingeventlistener.h"

#include "core/pipeline/abstractprocessordecorator.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/allproperties.h"

#include "modules/base/processors/trackballcameraprovider.h"
#include "modules/modulesapi.h"
#include "modules/vis/processors/volumerenderer.h"
#include "modules/vis/processors/sliceextractor.h"
#include "../../processorfactory.h"

namespace cgt {
    class Shader;
}

class FaceGeometry;

namespace campvis {
    /**
     * Combines a volume raycaster and 3 slice views for explorative volume visualization.
     */
    class CAMPVIS_MODULES_API VolumeExplorer : public VisualizationProcessor, public HasProcessorDecorators, public cgt::EventListener {
    public:
        /// Enumeration of the 4 views of the VolumeExplorer
        enum Views {
            XY_PLANE,
            XZ_PLANE,
            YZ_PLANE,
            VOLUME
        };

        /**
         * Constructs a new VolumeExplorer Processor
         * \param   viewportSizeProp    Pointer to the property defining the viewport size, must not be 0.
         * \param   raycaster           Raycaster to use for rendering, must not be 0, VolumeRenderer will take ownership.
         **/
        VolumeExplorer(IVec2Property* viewportSizeProp, SliceRenderProcessor* sliceRenderer = new SliceExtractor(0), RaycastingProcessor* raycaster = new SimpleRaycaster(0));

        /**
         * Destructor
         **/
        virtual ~VolumeExplorer();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /** 
         * To be used in ProcessorFactory static methods
         */
        static const std::string getId() { return "VolumeExplorer"; };
        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return getId(); };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Combines a volume raycaster and 3 slice views for explorative volume visualization."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };
        
        /// \see cgt::EventListener::onEvent()
        virtual void onEvent(cgt::Event* e);

        /**
         * Returns the VolumeRenderer processor.
         * \return	&_raycaster
         */
        VolumeRenderer* getVolumeRenderer();

        /**
         * Returns the SliceRenderProcessor.
         * \return	_sliceRenderer
         */
        SliceRenderProcessor* getSliceRenderer();

        DataNameProperty p_inputVolume;     ///< image ID for first input image
        DataNameProperty p_outputImage;     ///< image ID for output image

        GenericOptionProperty<Views> p_largeView;   ///< View to be shown in the large render target
        BoolProperty p_enableScribbling;    ///< Enable Scribbling in Slice Views

        MetaProperty p_seProperties;        ///< MetaProperty for SliceExtractor properties
        MetaProperty p_vrProperties;        ///< MetaProperty for Raycaster properties


    protected:
        /// Additional invalidation levels for this processor.
        /// Not the most beautiful design though.
        enum ProcessorInvalidationLevel {
            CAMERA_INVALID = FIRST_FREE_TO_USE_INVALIDATION_LEVEL,
            VR_INVALID = FIRST_FREE_TO_USE_INVALIDATION_LEVEL << 1,
            SLICES_INVALID = FIRST_FREE_TO_USE_INVALIDATION_LEVEL << 2,
            SCRIBBLE_INVALID = FIRST_FREE_TO_USE_INVALIDATION_LEVEL << 3,
            LARGE_VIEW_INVALID = FIRST_FREE_TO_USE_INVALIDATION_LEVEL << 4
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
        void onSliceExtractorScribblePainted(cgt::vec3 voxel);

        void composeFinalRendering(DataContainer& data);

        /// \see    AbstractProcessor::updateProperties
        void updateProperties(DataContainer& dc);

        cgt::Shader* _shader;                           ///< Shader for slice rendering
        FaceGeometry* _quad;

        TrackballCameraProvider _tcp;
        VolumeRenderer _raycaster;
        SliceRenderProcessor* _sliceRenderer;

        IVec2Property p_smallRenderSize;
        IVec2Property p_largeRenderSize;


        MWheelToNumericPropertyEventListener _xSliceHandler;
        MWheelToNumericPropertyEventListener _ySliceHandler;
        MWheelToNumericPropertyEventListener _zSliceHandler;
        TransFuncWindowingEventListener _windowingHandler;
        bool _mousePressedInRaycaster;                  ///< Flag whether mouse was pressed in raycaster

        Views _viewUnderEvent;                          ///< View to apply events to
        cgt::ivec2 _eventPositionOffset;                ///< Offset to be added to mouse event position
        cgt::ivec2 _eventViewportSize;                  ///< Viewport of adjusted mouse event

        std::vector<cgt::vec3>* _scribblePointer;       ///< Pointer encoding whether the mouse was pressed (!= nullptr) and whether we have yes-scribbles or no-scribbles.
        std::vector<cgt::vec3> _yesScribbles;           ///< All voxels of the current yes-scribbles
        std::vector<cgt::vec3> _noScribbles;            ///< All voxels of the current no-scribbles

        cgt::ivec3 _cachedImageSize;

        static const std::string loggerCat_;
    };

    // Instantiate template to register the pipelines.
    template class SmartProcessorRegistrar<VolumeExplorer>;
}

#endif // VOLUMEEXPLORER_H__
