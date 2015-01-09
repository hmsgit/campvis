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

#include "streamingmhddemo.h"

#include "cgt/event/keyevent.h"
#include "core/datastructures/imagedata.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    StreamingMhdDemo::StreamingMhdDemo(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _imageReader()
        , _ve(&_canvasSize)
    {
        addProcessor(&_imageReader);
        addProcessor(&_ve);

        addEventListenerToBack(&_ve);
    }

    StreamingMhdDemo::~StreamingMhdDemo() {
    }

    void StreamingMhdDemo::init() {
        AutoEvaluationPipeline::init();
        
        _imageReader.s_validated.connect(this, &StreamingMhdDemo::onProcessorValidated);

        _ve.p_outputImage.setValue("combine");
        _renderTargetID.setValue("combine");

        _imageReader.p_address.setValue("127.0.0.1");
        _imageReader.p_targetImageID.setValue("reader.output");
        _imageReader.p_targetImageID.addSharedProperty(&_ve.p_inputVolume);

        Geometry1DTransferFunction* dvrTF = new Geometry1DTransferFunction(128, cgt::vec2(0.f, 1.f));
        dvrTF->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.1f, .125f), cgt::col4(255, 0, 0, 32), cgt::col4(255, 0, 0, 32)));
        dvrTF->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.4f, .5f), cgt::col4(0, 255, 0, 128), cgt::col4(0, 255, 0, 128)));
        static_cast<TransferFunctionProperty*>(_ve.getProperty("TransferFunction"))->replaceTF(dvrTF);

        _canvasSize.s_changed.connect<StreamingMhdDemo>(this, &StreamingMhdDemo::onRenderTargetSizeChanged);
    }

    void StreamingMhdDemo::deinit() {
        _canvasSize.s_changed.disconnect(this);
        AutoEvaluationPipeline::deinit();
    }

    void StreamingMhdDemo::onRenderTargetSizeChanged(const AbstractProperty* prop) {
    }

    void StreamingMhdDemo::onProcessorValidated(AbstractProcessor* processor) {
    }


}