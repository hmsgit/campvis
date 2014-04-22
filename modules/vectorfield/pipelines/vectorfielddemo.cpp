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

#include "vectorfielddemo.h"

#include "tgt/event/keyevent.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    VectorFieldDemo::VectorFieldDemo(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _lsp()
        , _imageReader()
        , _vectorFieldReader()
        , _vectorFieldRenderer(&_canvasSize)
        , _sliceRenderer(&_canvasSize)
        , _rtc(&_canvasSize)
        , p_camera("Camera", "Camera", tgt::Camera())
        , p_sliceNumber("SliceNuber", "Slice Number", 0, 0, 1024)
        , _trackballEH(0)

    {
        addProperty(p_camera);
        addProperty(p_sliceNumber);

        _trackballEH = new TrackballNavigationEventListener(&p_camera, &_canvasSize);
        addEventListenerToBack(_trackballEH);

        addProcessor(&_lsp);
        addProcessor(&_imageReader);
        addProcessor(&_vectorFieldReader);
        addProcessor(&_vectorFieldRenderer);
        addProcessor(&_sliceRenderer);
        addProcessor(&_rtc);
    }

    VectorFieldDemo::~VectorFieldDemo() {
    }

    void VectorFieldDemo::init() {
        AutoEvaluationPipeline::init();

        p_camera.addSharedProperty(&_vectorFieldRenderer.p_camera);
        p_camera.addSharedProperty(&_sliceRenderer.p_camera);

        p_sliceNumber.addSharedProperty(&_vectorFieldRenderer.p_sliceNumber);
        p_sliceNumber.addSharedProperty(&_sliceRenderer.p_sliceNumber);

        _imageReader.p_url.setValue(CAMPVIS_SOURCE_DIR "/modules/vectorfield/sampledata/rec1_2D_comp.mhd");
        
        _imageReader.p_targetImageID.setValue("reader.output");
        _imageReader.p_targetImageID.addSharedProperty(&_sliceRenderer.p_sourceImageID);
        _imageReader.s_validated.connect(this, &VectorFieldDemo::onProcessorValidated);

        _vectorFieldReader.p_url.setValue(CAMPVIS_SOURCE_DIR "/modules/vectorfield/sampledata/result_vec.mhd");
        _vectorFieldReader.p_targetImageID.setValue("vectors");
        _vectorFieldReader.p_targetImageID.addSharedProperty(&_vectorFieldRenderer.p_inputVectors);

        _vectorFieldRenderer.p_renderOutput.addSharedProperty(&_rtc.p_firstImageId);
        _vectorFieldRenderer.p_arrowSize.setValue(0.03f);
        _vectorFieldRenderer.p_lenThresholdMin.setValue(100.f);
        _vectorFieldRenderer.p_flowProfile1.setValue(0.4716088614374652f);
        _vectorFieldRenderer.p_flowProfile2.setValue(0.0638348311845516f);
        _vectorFieldRenderer.p_flowProfile3.setValue(0.1713471562960614f);
        _vectorFieldRenderer.p_flowProfile4.setValue(0.1019371804834016f);
        _vectorFieldRenderer.p_lenThresholdMax.setValue(400.f);
        _vectorFieldRenderer.p_sliceOrientation.setValue(3);

        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(128, tgt::vec2(0.f, 1.f));
        tf->addGeometry(TFGeometry1D::createQuad(tgt::vec2(0.f, 1.f), tgt::col4(0, 0, 0, 255), tgt::col4(255, 255, 255, 255)));
        _sliceRenderer.p_transferFunction.replaceTF(tf);
        _sliceRenderer.p_targetImageID.setValue("slice");
        _sliceRenderer.p_targetImageID.addSharedProperty(&_rtc.p_secondImageId);

        _rtc.p_compositingMethod.selectById("depth");
        _rtc.p_targetImageId.setValue("composed");

        _renderTargetID.setValue("composed");
    }

    void VectorFieldDemo::onProcessorValidated(AbstractProcessor* processor) {
        if (processor == &_imageReader) {
            // update camera
            ScopedTypedData<IHasWorldBounds> img(*_data, _sliceRenderer.p_sourceImageID.getValue());
            if (img) {
                _trackballEH->reinitializeCamera(img);
            }
        }
    }

}
