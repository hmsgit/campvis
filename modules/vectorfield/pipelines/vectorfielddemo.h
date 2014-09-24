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

#ifndef VECTORFIELDDEMO_H__
#define VECTORFIELDDEMO_H__

#include "core/pipeline/autoevaluationpipeline.h"

#include "core/eventhandlers/mwheeltonumericpropertyeventlistener.h"
#include "core/eventhandlers/trackballnavigationeventlistener.h"

#include "modules/base/processors/lightsourceprovider.h"
#include "modules/io/processors/mhdimagereader.h"
#include "modules/vectorfield/processors/particleflowrenderer.h"
#include "modules/vectorfield/processors/vectorfieldrenderer.h"
#include "modules/vis/processors/slicerenderer3d.h"
#include "modules/vis/processors/rendertargetcompositor.h"

namespace campvis {
    class VectorFieldDemo : public AutoEvaluationPipeline {
    public:
        /**
         * Small demo pipeline for vector field visualization.
         */
        VectorFieldDemo(DataContainer* dc);

        /**
         * Virtual Destructor
         **/
        virtual ~VectorFieldDemo();

        /// \see AutoEvaluationPipeline::init()
        virtual void init();

        /// \see AbstractPipeline::getName()
        virtual const std::string getName() const { return getId(); };
        /// \see AbstractPipeline::getId()
        static const std::string getId() { return "VectorFieldDemo"; };

    protected:
        /**
         * Slot getting called when one of the observed processors got validated.
         * Updates the camera properties, when the input image has changed.
         * \param   processor   The processor that emitted the signal
         */
        virtual void onProcessorValidated(AbstractProcessor* processor);

        LightSourceProvider _lsp;
        MhdImageReader _imageReader;
        MhdImageReader _vectorFieldReader;
        ParticleFlowRenderer _pfr;
        VectorFieldRenderer _vectorFieldRenderer;
        SliceRenderer3D _sliceRenderer;
        RenderTargetCompositor _rtc;

        CameraProperty p_camera;
        IntProperty p_sliceNumber;
        TrackballNavigationEventListener* _trackballEH;
    };

}

#endif // VECTORFIELDDEMO_H__
