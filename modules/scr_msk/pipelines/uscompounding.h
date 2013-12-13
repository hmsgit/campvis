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

#ifndef USCOMPOUNDING_H__
#define USCOMPOUNDING_H__


#include "core/datastructures/imagerepresentationlocal.h"
#include "core/eventhandlers/trackballnavigationeventlistener.h"
#include "core/pipeline/autoevaluationpipeline.h"
#include "core/properties/cameraproperty.h"

#include "modules/scr_msk/processors/trackedusfilereader.h"
#include "modules/scr_msk/processors/trackedussweepframerenderer3d.h"

namespace campvis {
    class UsCompounding : public AutoEvaluationPipeline {
    public:
        /**
         * Creates a AutoEvaluationPipeline. 
         */
        UsCompounding(DataContainer* dc);

        /**
         * Virtual Destructor
         **/
        virtual ~UsCompounding();

        /// \see AutoEvaluationPipeline::init()
        virtual void init();

        /// \see AutoEvaluationPipeline::deinit()
        virtual void deinit();

        /// \see AbstractPipeline::getName()
        virtual const std::string getName() const { return getId(); };
        static const std::string getId() { return "UsCompounding"; };
        
    protected:
        void onBoundingBoxChanged(tgt::Bounds b);

        CameraProperty p_camera;

        TrackedUsFileReader _reader;
        TrackedUsSweepFrameRenderer3D _renderer;

        TrackballNavigationEventListener* _trackballEH;
    };

}

#endif // USCOMPOUNDING_H__
