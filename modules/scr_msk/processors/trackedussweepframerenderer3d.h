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

#ifndef TRACKEDUSSWEEPFRAMERENDERER3D_H__
#define TRACKEDUSSWEEPFRAMERENDERER3D_H__

#include <string>

#include "tgt/bounds.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/buttonproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/transferfunctionproperty.h"
#include "core/properties/cameraproperty.h"

class TrackedUSFileIO;
class TrackedUSSweep;

namespace tgt {
    class Shader;
}

namespace campvis {
    class ImageData;

    /**
     * Extracts a slice from a 3D image and renders it into a rendertarget.
     */
    class TrackedUsSweepFrameRenderer3D : public VisualizationProcessor {
    public:
        /**
         * Constructs a new TrackedUsSweepFrameRenderer3D Processor
         **/
        TrackedUsSweepFrameRenderer3D(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~TrackedUsSweepFrameRenderer3D();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "TrackedUsSweepFrameRenderer3D"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Extracts a single slice from the input image and renders it using a transfer function."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        /// \see AbstractProcessor::process()
        virtual void process(DataContainer& data);

        DataNameProperty p_sourceImageID;                  ///< image ID for input FileIO
        DataNameProperty p_targetImageID;                  ///< image ID for output image

        CameraProperty p_camera;
        IntProperty p_sweepNumber;                          ///< Number of the sweep to display
        IntProperty p_frameNumber;                          ///< Number of the frame to display
        BoolProperty p_showConfidenceMap;                   ///< Flag whether to show confidence map instead of US image
        ButtonProperty p_smoothButton;

        TransferFunctionProperty p_transferFunction;     ///< Transfer function

        /// adapts the range of the p_frameNumber property to the image
        virtual void updateProperties(DataContainer& dc);

        const TrackedUSSweep* getCurrentSweep() const;

        sigslot::signal1<tgt::Bounds> s_boundingBoxChanged;

    protected:
        void onSmoothButtonClicked();

        void updateBoundingBox();

        tgt::Shader* _shader;                           ///< Shader for slice rendering
        TrackedUSSweep* _currentSweep;
        tgt::Bounds _bounds;

        static const std::string loggerCat_;
    };
}

#endif // TRACKEDUSSWEEPFRAMERENDERER3D_H__
