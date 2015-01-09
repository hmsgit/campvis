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

#ifndef IXPVDEMO_H__
#define IXPVDEMO_H__

#include "core/datastructures/imagerepresentationlocal.h"
#include "core/eventhandlers/mwheeltonumericpropertyeventlistener.h"
#include "core/pipeline/autoevaluationpipeline.h"

#include "modules/modulesapi.h"
#include "modules/pipelinefactory.h"

#include "modules/base/processors/trackballcameraprovider.h"
#include "modules/io/processors/mhdimagereader.h"
#include "modules/devil/processors/devilimagereader.h"
#include "modules/vis/processors/sliceextractor.h"
#include "modules/vis/processors/proxygeometrygenerator.h"
#include "modules/vis/processors/geometryrenderer.h"
#include "modules/vis/processors/eepgenerator.h"
#include "modules/vis/processors/drrraycaster.h"
#include "modules/vis/processors/simpleraycaster.h"
#include "modules/vis/processors/rendertargetcompositor.h"
#include "modules/vis/processors/slicerenderer3d.h"
#include "modules/ixpv/processors/ixpvcompositor.h"
#include "modules/vis/processors/volumerenderer.h"



namespace campvis {
    class CAMPVIS_MODULES_API IxpvDemo : public AutoEvaluationPipeline {
    public:
        /**
         * Creates a AutoEvaluationPipeline.
         */
        IxpvDemo(DataContainer* dc);

        /**
         * Virtual Destructor
         **/
        virtual ~IxpvDemo();

        /// \see AutoEvaluationPipeline::init()
        virtual void init();

        /// \see AbstractPipeline::getName()
        virtual const std::string getName() const { return getId(); };
        static const std::string getId() { return "IxpvDemo"; };

        //virtual void keyEvent(cgt::KeyEvent* e);
    protected:
        /**
         * Slot getting called when one of the observed processors got validated.
         * Updates the camera properties, when the input image has changed.
         * \param   processor   The processor that emitted the signal
         */
        virtual void onProcessorValidated(AbstractProcessor* processor);

        TrackballCameraProvider _tcp;
        DevilImageReader _xrayReader;

        MhdImageReader _ctReader;

        VolumeRenderer _vrFull;
        VolumeRenderer _vrClipped;

        MhdImageReader _usReader;
        SliceRenderer3D _usSliceRenderer;

        RenderTargetCompositor _compositor;
        IxpvCompositor _ixpvCompositor;

        MWheelToNumericPropertyEventListener _wheelHandler;
    };

    // Instantiate template to register the pipelines.
    template class PipelineRegistrar<IxpvDemo>;

}

#endif // IXPVDEMO_H__

