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

#ifndef MANUALTISSUESEGMENTER_H__
#define MANUALTISSUESEGMENTER_H__

#include <string>
#include <map>

#include "cgt/bounds.h"
#include "cgt/event/eventlistener.h"

#include "core/datastructures/datahandle.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/allproperties.h"

#include "modules/modulesapi.h"
#include "modules/manualsegmentation/tools/tissuesegmentation.h"

namespace cgt {
    class Shader;
}

namespace campvis {
    class ImageData;

    /**
     * VERY EXPERIMENTAL processor for manual segmentation of tissue layers.
     */
    class CAMPVIS_MODULES_API ManualTissueSegmenter : public VisualizationProcessor, public cgt::EventListener {
    public:
        /**
         * Constructs a new ManualTissueSegmenter Processor
         **/
        ManualTissueSegmenter(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~ManualTissueSegmenter();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractEventHandler::execute()
        virtual void onEvent(cgt::Event* e);

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "ManualTissueSegmenter"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "VERY EXPERIMENTAL processor for manual segmentation of tissue layers."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_sourceImageID;                  ///< image ID for input FileIO
        DataNameProperty p_targetImageID;                  ///< image ID for output image

        IntProperty p_frameNumber;                          ///< Number of the frame to display
        TransferFunctionProperty p_transferFunction;        ///< Transfer function

        BoolProperty p_fullWidth;
        IntProperty p_splineOrder;
        IntProperty p_NumControlPoints;
        IntProperty p_NumLevels;

        ButtonProperty p_computeSamples;
        BoolProperty p_showSamples;
        StringProperty p_sampleFile;
        ButtonProperty p_saveSamples;

        StringProperty p_controlpointFile;
        ButtonProperty p_saveCPs;
        ButtonProperty p_loadCPs;

        IntProperty p_activeLayer;
        ButtonProperty p_addLayer;

        ButtonProperty p_exportToLabelImage;

    protected:
        struct ControlPoint {
            cgt::vec2 _pixel;
        };

        /// \see AbstractProcessor::updateResult()
        virtual void updateResult(DataContainer& data);

        /// adapts the range of the p_frameNumber property to the image
        virtual void updateProperties(DataContainer& data);

        /// \see AbstractProcessor::onPropertyChanged
        virtual void onPropertyChanged(const AbstractProperty* prop);

        void computeSpline();

        void computeSamples();

        void saveSamples();

        void saveControlPoints();

        void loadControlPoints();

        void onAddLayer();

        cgt::Shader* _shader;                           ///< Shader for slice rendering

        bool _mousePressed;                 ///< Flag whether the mouse is currently pressed
        cgt::ivec2 _mouseDownPosition;      ///< Viewport coordinates where mouse button has been pressed
        ControlPoint* _currentControlPoint;
        cgt::vec4* _currentBounds;

        DataHandle _currentImage;
        std::vector< std::map< int, std::vector< ControlPoint > > > _controlPoints;
        std::map< int, std::vector< float > > _splines;

        std::map< int, cgt::vec4 > _vesselBounds;

        TissueSegmentation _segmentation;

        static const std::string loggerCat_;
    };
}

#endif // MANUALTISSUESEGMENTER_H__
