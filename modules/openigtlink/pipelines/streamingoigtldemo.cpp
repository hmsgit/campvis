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

#include "streamingoigtldemo.h"

#include "core/datastructures/imagedata.h"

namespace campvis {

    StreamingOIGTLDemo::StreamingOIGTLDemo(DataContainer& dc)
        : AutoEvaluationPipeline(dc, getId())
    {
        addProcessor(&_igtlClient);
        addProcessor(&_matrixProcessor);

        //addEventListenerToBack(&_ve);
    }

    StreamingOIGTLDemo::~StreamingOIGTLDemo() {
    }

    void StreamingOIGTLDemo::init() {
        AutoEvaluationPipeline::init();
        
        _igtlClient.s_validated.connect(this, &StreamingOIGTLDemo::onProcessorValidated);
        _matrixProcessor.s_validated.connect(this, &StreamingOIGTLDemo::onProcessorValidated);

        _renderTargetID.setValue("IGTL.image.ImagerClient");
        _matrixProcessor.p_matrixAID.setValue("IGTL.transform.ProbeToTracker");
        _matrixProcessor.p_matrixAType.selectByOption("data");
        _matrixProcessor.p_matrixBID.setValue("IGTL.transform.ReferenceToTracker");
        _matrixProcessor.p_matrixBType.selectByOption("data");

        _igtlClient.p_address.setValue("127.0.0.1");

        _canvasSize.s_changed.connect<StreamingOIGTLDemo>(this, &StreamingOIGTLDemo::onRenderTargetSizeChanged);
    }

    void StreamingOIGTLDemo::deinit() {
        _canvasSize.s_changed.disconnect(this);
        AutoEvaluationPipeline::deinit();
    }

    void StreamingOIGTLDemo::onRenderTargetSizeChanged(const AbstractProperty *prop) {
    }

    void StreamingOIGTLDemo::onProcessorValidated(AbstractProcessor *processor) {
    }


}