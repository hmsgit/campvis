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

#ifndef MICROSCOPYIMAGESEGMENTATION_H__
#define MICROSCOPYIMAGESEGMENTATION_H__

#include "cgt/event/eventlistener.h"

#include "core/eventhandlers/mwheeltonumericpropertyeventlistener.h"
#include "core/eventhandlers/transfuncwindowingeventlistener.h"

#include "core/pipeline/abstractprocessordecorator.h"
#include "core/pipeline/visualizationprocessor.h"

#include "core/properties/allproperties.h"
#include "core/properties/colorproperty.h"
#include "core/datastructures/facegeometry.h"

#include "modules/modulesapi.h"
#include "modules/base/processors/trackballcameraprovider.h"
#include "modules/vis/processors/volumerenderer.h"
#include "modules/vis/processors/sliceextractor.h"

#include <vector>
#include <memory>
#include "core/datastructures/facegeometry.h"

namespace cgt {
    class Shader;
}

//class FaceGeometry;

namespace campvis {    
    /**
     * Structure to store the informations related to a contour object.
     */
    class CAMPVIS_MODULES_API ContourObject {
    public:
        explicit ContourObject(std::string name);
        virtual ~ContourObject();
        ContourObject& operator=(const ContourObject& rhs);
        void addObject();

        StringProperty _objectName;
        ColorProperty _color;                       ///< Color of the Object
        BoolProperty _visibility;
        std::vector<cgt::ivec3> _points;            ///< List of points for the current object being painted

        std::vector< std::vector<cgt::ivec3> > _objectsCoordinates;         ///< List of previously painted contours
    protected:
        enum {
            CONTOUR = 0,
            PAINTED = 1,
        };

    private:
    };
    
    /**
     * Defines a proxy FaceGeometry to pass to SliceExtractor and render()
     * Thus also overloads the render() function.
     */
    class CAMPVIS_MODULES_API ProxyFaceGeometry : public FaceGeometry {
    public:
        void addGeometry(FaceGeometry geometry);
        void render(GLenum mode) const;
    protected:

    private:
        std::vector<FaceGeometry> _geometries;
    };


    /**
     * Combines a volume raycaster and 3 slice views for graphical segmentation of slices through user input.
     */
    class CAMPVIS_MODULES_API MicroscopyImageSegmentation : public VisualizationProcessor, public HasProcessorDecorators, public cgt::EventListener {
    public:
        /**
         * Constructs a new MicroscopyImageSegmentation Processor
         * \param   viewportSizeProp    Pointer to the property defining the viewport size, must not be 0.
         * \param   raycaster           Raycaster to use for rendering, must not be 0, VolumeRenderer will take ownership.
         **/
        MicroscopyImageSegmentation(IVec2Property* viewportSizeProp, RaycastingProcessor* raycaster = new SimpleRaycaster(0));

        /**
         * Destructor
         **/
        virtual ~MicroscopyImageSegmentation();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();
        
        /** 
         * To be used in ProcessorFactory static methods
         */
        static const std::string getId() { return "MicroscopyImageSegmentation"; };
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
            AXISSCALE_INVALID = FIRST_FREE_TO_USE_INVALIDATION_LEVEL << 3,
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
        std::unique_ptr<FaceGeometry> _quad;

        TrackballCameraProvider _tcp;
        VolumeRenderer _vr;
        SliceExtractor _sliceExtractor;

        IVec2Property p_rightPaneBlockSize;             ///< Used for defining space for volume explorer, xslice, yslice
        IVec2Property p_leftPaneSize;                   ///< Used to define space for zslice
        IVec2Property p_zSize;                          ///< Actual size for zslice

        MWheelToNumericPropertyEventListener _xSliceHandler;
        MWheelToNumericPropertyEventListener _ySliceHandler;
        MWheelToNumericPropertyEventListener _zSliceHandler;
        TransFuncWindowingEventListener _windowingHandler;

        ColorProperty p_paintColor;                     ///< Color picker for active object
        Vec3Property p_axisScaling;                     ///< Asis ratio. Used to change ImageMappingInformation
        cgt::vec3 _oldScaling;

        BoolProperty p_fitToWindow;                         ///< Flag whether fit image to window or use scaling and offset
        FloatProperty p_scalingFactor;                      ///< Image scaling factor
        IVec2Property p_offset;                             ///< Image offset

        ButtonProperty p_addObject;                     ///< Add a contour object
        ButtonProperty p_deleteObject;                  ///< Delete the current object
        StringProperty p_csvPath;                       ///< Filepath to save CSV
        ButtonProperty p_saveCSV;                       ///< Save changes in CSV
        GenericOptionProperty<ContourObject> *p_objectList; ///< List of drawn contours
        bool _mousePressedInRaycaster;                  ///< Flag whether mouse was pressed in raycaster

        static const std::string loggerCat_;

    private:
        void onAddButtonClicked();
        void onDeleteButtonClicked();
        void onSaveCSVButtonClicked();
        void onCSVFileSelected(const AbstractProperty *prop);
        void onObjectSelectionChanged(const AbstractProperty *prop);
        void onPaintColorChanged(const AbstractProperty *prop);

        void addProperties();
        void removeProperties();

        double distanceSqr(cgt::vec3 src, cgt::vec3 dest);

        std::string _objectNamePrefix;
        int _objectNameSuffix;
        bool editVoxel;
        int insertNextVoxelAt;
    };
}

#endif // MICROSCOPYIMAGESEGMENTATION_H__
