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

#include "uscompounding.h"


namespace campvis {


    UsCompounding::UsCompounding(DataContainer* dc) 
        : AutoEvaluationPipeline(dc)
        , p_camera("Camera", "Camera")
        , _reader()
        , _renderer(&_canvasSize)
        , _trackballEH(0)
    {
        addProcessor(&_reader);
        addProcessor(&_renderer);

        addProperty(&p_camera);

        _trackballEH = new TrackballNavigationEventListener(&p_camera, &_canvasSize);
        addEventListenerToBack(_trackballEH);
    }

    UsCompounding::~UsCompounding() {
        delete _trackballEH;
    }

    void UsCompounding::init() {
        AutoEvaluationPipeline::init();

        p_camera.addSharedProperty(&_renderer.p_camera);

        _reader.p_url.setValue("C:/Users/SchuCh01/Documents/Data/Ultrasound/2012-12-12-Test/9l4sweep/content.xml");
        _reader.p_targetImageID.addSharedProperty(&_renderer.p_sourceImageID);

        _renderer.s_boundingBoxChanged.connect(this, &UsCompounding::onBoundingBoxChanged);
        _renderer.p_targetImageID.setValue("us.frame.output");
        _renderTargetID.setValue(_renderer.p_targetImageID.getValue());
    }

    void UsCompounding::deinit() {
        _canvasSize.s_changed.disconnect(this);
        AutoEvaluationPipeline::deinit();
    }

    void UsCompounding::onBoundingBoxChanged(tgt::Bounds b) {
        _trackballEH->reinitializeCamera(b);
    }

}