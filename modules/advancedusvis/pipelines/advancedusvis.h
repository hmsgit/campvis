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

#ifndef ADVANCEDUSVIS_H__
#define ADVANCEDUSVIS_H__

#include "core/datastructures/imagerepresentationlocal.h"
#include "core/eventhandlers/mwheeltonumericpropertyeventlistener.h"
#include "core/eventhandlers/transfuncwindowingeventlistener.h"
#include "core/eventhandlers/trackballnavigationeventlistener.h"
#include "core/pipeline/autoevaluationpipeline.h"
#include "modules/devil/processors/devilimagereader.h"
#include "modules/io/processors/mhdimagereader.h"
#include "modules/io/processors/csvdimagereader.h"
#include "modules/advancedusvis/processors/advancedusfusion.h"
#include "modules/preprocessing/processors/gradientvolumegenerator.h"
#include "modules/preprocessing/processors/lhhistogram.h"
#include "modules/itk/processors/itkimagefilter.h"
#include "modules/vis/processors/proxygeometrygenerator.h"
#include "modules/vis/processors/eepgenerator.h"
#include "modules/vis/processors/simpleraycaster.h"
#include "modules/vis/processors/quadview.h"
#include "modules/randomwalk/processors/confidencemapgenerator.h"

namespace campvis {
    class AdvancedUsVis : public AutoEvaluationPipeline {
    public:
        /**
         * Creates a VisualizationPipeline. 
         */
        AdvancedUsVis(DataContainer* dc);

        /**
         * Virtual Destructor
         **/
        virtual ~AdvancedUsVis();

        /// \see VisualizationPipeline::init()
        virtual void init();

        /// \see VisualizationPipeline::deinit()
        virtual void deinit();

        /// \see AbstractPipeline::getName()
        virtual const std::string getName() const { return getId(); };
        static const std::string getId() { return "AdvancedUsVis"; };

        /**
         * Execute this pipeline.
         **/
        virtual void execute();

        virtual void keyEvent(tgt::KeyEvent* e);

    protected:

        /**
         * Slot getting called when one of the observed processors got validated.
         * Updates the camera properties, when the input image has changed.
         * \param   processor   The processor that emitted the signal
         */
        virtual void onProcessorValidated(AbstractProcessor* processor);

        CameraProperty _camera;

        MhdImageReader _usReader;
        MhdImageReader _confidenceReader;
        ConfidenceMapGenerator _confidenceGenerator;

        GradientVolumeGenerator _gvg;
        LHHistogram _lhh;
        AdvancedUsFusion _usFusion1;
        AdvancedUsFusion _usFusion2;
        AdvancedUsFusion _usFusion3;
        AdvancedUsFusion _usFusion4;
        ItkImageFilter _usBlurFilter;

        QuadView _quadView;

        ItkImageFilter _usDenoiseilter;
        ProxyGeometryGenerator _usProxy;
        EEPGenerator _usEEP;
        SimpleRaycaster _usDVR;

        MWheelToNumericPropertyEventListener _wheelHandler;
        TransFuncWindowingEventListener _tfWindowingHandler;

        TrackballNavigationEventListener* _trackballEH;
    };

}

#endif // ADVANCEDUSVIS_H__
