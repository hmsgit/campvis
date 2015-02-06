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

#ifndef VISCONTESTDEMO_H__
#define VISCONTESTDEMO_H__

#include "core/eventhandlers/mwheeltonumericpropertyeventlistener.h"
#include "core/pipeline/autoevaluationpipeline.h"
#include "core/pipeline/viewportsplitter.h"

#include "modules/modulesapi.h"
#include "modules/pipelinefactory.h"

#include "modules/base/processors/lightsourceprovider.h"
#include "modules/base/processors/trackballcameraprovider.h"
#include "modules/dti/processors/fiberreader.h"
#include "modules/dti/processors/fiberrenderer.h"
#include "modules/io/processors/genericimagereader.h"
#include "modules/neuro/processors/multivolumemprrenderer.h"
#include "modules/neuro/processors/multivolumeraycaster.h"
#include "modules/vis/processors/rendertargetcompositor.h"

namespace campvis {
    class CAMPVIS_MODULES_API ViscontestDemo : public AutoEvaluationPipeline {
    public:
        /**
         * Creates a AutoEvaluationPipeline.
         */
        ViscontestDemo(DataContainer* dc);

        /**
         * Virtual Destructor
         **/
        virtual ~ViscontestDemo();

        /// \see AutoEvaluationPipeline::init()
        virtual void init();
        virtual void deinit();

        /// \see AbstractPipeline::getName()
        virtual const std::string getName() const { return getId(); };
        /// \see AbstractPipeline::getId()
        static const std::string getId() { return "ViscontestDemo"; };

        /// \see AbstractPipeline::executePipeline()
        virtual void executePipeline();

    protected:
        void onReaderValidated(AbstractProcessor* p);
        
        void onSplitterEvent(size_t index, cgt::Event* e);

        LightSourceProvider _lsp;
        TrackballCameraProvider _tcp;

        dti::FiberReader _fiberReader;
        dti::FiberRenderer _fiberRenderer;

        GenericImageReader _t1PostReader;
        GenericImageReader _t1PreReader;
        GenericImageReader _flairReader;

        neuro::MultiVolumeMprRenderer _mvmpr2D;
        neuro::MultiVolumeMprRenderer _mvmpr3D;
        neuro::MultiVolumeRaycaster _mvr;

        RenderTargetCompositor _rtc1;
        RenderTargetCompositor _rtc2;

        ViewportSplitter _horizontalSplitter;

        MWheelToNumericPropertyEventListener _slicePositionEventHandler;

    };

    // Instantiate template to register the pipelines.
    template class PipelineRegistrar<ViscontestDemo>;

}

#endif // VISCONTESTDEMO_H__
