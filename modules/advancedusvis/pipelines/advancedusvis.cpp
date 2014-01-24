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

#include "advancedusvis.h"

#include "tgt/event/keyevent.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"
#include "core/tools/simplejobprocessor.h"
#include "core/tools/job.h"

namespace campvis {

    AdvancedUsVis::AdvancedUsVis(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _camera("camera", "Camera")
        , _usReader()
        , _confidenceReader()
        , _confidenceGenerator()
        , _gvg()
        , _lhh()
        , _usFusion1(&_canvasSize)
        , _usFusion2(&_canvasSize)
        , _usFusion3(&_canvasSize)
        , _usFusion4(&_canvasSize)
        , _usBlurFilter()
        , _quadView(&_canvasSize)
        , _usDenoiseilter()
        , _usProxy()
        , _usEEP(&_canvasSize)
        , _usDVR(&_canvasSize)
        , _wheelHandler(&_usFusion1.p_sliceNumber)
        , _tfWindowingHandler(&_usFusion1.p_transferFunction)
        , _trackballEH(0)
    {
        addProcessor(&_usReader);
        addProcessor(&_confidenceReader);
        //addProcessor(&_confidenceGenerator);
        //addProcessor(&_gvg);
        //addProcessor(&_lhh);
        addProcessor(&_usBlurFilter);
        addProcessor(&_usFusion1);
        addProcessor(&_usFusion2);
        addProcessor(&_usFusion3);
        addProcessor(&_usFusion4);
        addProcessor(&_quadView);
        //addProcessor(&_usDenoiseilter);
        //addProcessor(&_usProxy);
        //addProcessor(&_usEEP);
        //addProcessor(&_usDVR);

        addEventListenerToBack(&_wheelHandler);
        //addEventHandler(&_tfWindowingHandler);

        _trackballEH = new TrackballNavigationEventListener(&_camera, &_canvasSize);
        addEventListenerToBack(_trackballEH);
    }

    AdvancedUsVis::~AdvancedUsVis() {
        delete _trackballEH;
    }

    void AdvancedUsVis::init() {
        AutoEvaluationPipeline::init();

        _usReader.s_validated.connect(this, &AdvancedUsVis::onProcessorValidated);

        _camera.addSharedProperty(&_usEEP.p_camera);
        _camera.addSharedProperty(&_usDVR.p_camera);

        _usReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\CurefabCS\\Stent_Patient_ B-Mode_2013-02-11T14.56.46z\\01_us.mhd");
        //_usReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\01\\BMode_01.mhd");
        //_usReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\UltrasoundBoneData\\SynthesEvaluationUnterschenkel");
        //_usReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\transcranial\\us.png");
        _usReader.p_targetImageID.setValue("us.image");
        _usReader.p_targetImageID.addSharedProperty(&_confidenceGenerator.p_sourceImageID);
        _usReader.p_targetImageID.addSharedProperty(&_usFusion1.p_usImageId);
        _usReader.p_targetImageID.addSharedProperty(&_usFusion2.p_usImageId);
        _usReader.p_targetImageID.addSharedProperty(&_usFusion3.p_usImageId);
        _usReader.p_targetImageID.addSharedProperty(&_usFusion4.p_usImageId);
        _usReader.p_targetImageID.addSharedProperty(&_gvg.p_sourceImageID);
        _usReader.p_targetImageID.addSharedProperty(&_lhh.p_intensitiesId);
        _usReader.p_targetImageID.addSharedProperty(&_usBlurFilter.p_sourceImageID);
        _usReader.p_targetImageID.addSharedProperty(&_usDenoiseilter.p_sourceImageID);

        _confidenceReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\CurefabCS\\Stent_Patient_ B-Mode_2013-02-11T14.56.46z\\01_cm.mhd");
        //_confidenceReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\01\\Confidence_01.mhd");
        //_confidenceReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\UltrasoundBoneData\\SynthesEvaluationUnterschenkel");
        //_confidenceReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\transcranial\\cm.png");
        _confidenceReader.p_targetImageID.setValue("confidence.image.read");
        _confidenceReader.p_targetImageID.addSharedProperty(&_usFusion1.p_confidenceImageID);
        _confidenceReader.p_targetImageID.addSharedProperty(&_usFusion2.p_confidenceImageID);
        _confidenceReader.p_targetImageID.addSharedProperty(&_usFusion3.p_confidenceImageID);
        _confidenceReader.p_targetImageID.addSharedProperty(&_usFusion4.p_confidenceImageID);

        _confidenceGenerator.p_targetImageID.setValue("confidence.image.generated");
        //_confidenceGenerator.p_targetImageID.addSharedProperty(&_usFusion1.p_confidenceImageID);
        //_confidenceGenerator.p_targetImageID.addSharedProperty(&_usFusion2.p_confidenceImageID);
        //_confidenceGenerator.p_targetImageID.addSharedProperty(&_usFusion3.p_confidenceImageID);
        //_confidenceGenerator.p_targetImageID.addSharedProperty(&_usFusion4.p_confidenceImageID);

        _gvg.p_targetImageID.addSharedProperty(&_lhh.p_gradientsId);
        _gvg.p_targetImageID.addSharedProperty(&_usFusion1.p_gradientImageID);
        _gvg.p_targetImageID.addSharedProperty(&_usFusion2.p_gradientImageID);
        _gvg.p_targetImageID.addSharedProperty(&_usFusion3.p_gradientImageID);
        _gvg.p_targetImageID.addSharedProperty(&_usFusion4.p_gradientImageID);

        _usFusion1.p_targetImageID.setValue("us.fused1");
        _usFusion1.p_targetImageID.addSharedProperty(&_quadView.p_inputImage1);
        _usFusion1.p_view.selectById("us");
        _usFusion1.p_sliceNumber.setValue(0);
        _usFusion1.p_sliceNumber.addSharedProperty(&_usFusion2.p_sliceNumber);
        _usFusion1.p_sliceNumber.addSharedProperty(&_usFusion3.p_sliceNumber);
        _usFusion1.p_sliceNumber.addSharedProperty(&_usFusion4.p_sliceNumber);

        _usFusion2.p_targetImageID.setValue("us.fused2");
        _usFusion2.p_targetImageID.addSharedProperty(&_quadView.p_inputImage2);
        _usFusion2.p_view.selectById("mappingSaturationHSV");

        _usFusion3.p_targetImageID.setValue("us.fused3");
        _usFusion3.p_targetImageID.addSharedProperty(&_quadView.p_inputImage3);
        _usFusion3.p_view.selectById("mappingLAB");
        _usFusion3.p_hue.setValue(0.22f);

        _usFusion4.p_targetImageID.setValue("us.fused4");
        _usFusion4.p_targetImageID.addSharedProperty(&_quadView.p_inputImage4);
        _usFusion4.p_view.selectById("mappingSharpness");

        _usBlurFilter.p_targetImageID.setValue("us.blurred");
        _usBlurFilter.p_targetImageID.addSharedProperty(&_usFusion1.p_blurredImageId);
        _usBlurFilter.p_targetImageID.addSharedProperty(&_usFusion2.p_blurredImageId);
        _usBlurFilter.p_targetImageID.addSharedProperty(&_usFusion3.p_blurredImageId);
        _usBlurFilter.p_targetImageID.addSharedProperty(&_usFusion4.p_blurredImageId);
        _usBlurFilter.p_filterMode.selectById("gauss");
        _usBlurFilter.p_sigma.setValue(4.f);

        _quadView.p_outputImage.setValue("quadview.output");

        _usDenoiseilter.p_targetImageID.setValue("us.denoised");
        _usDenoiseilter.p_targetImageID.addSharedProperty(&_usProxy.p_sourceImageID);
        _usDenoiseilter.p_targetImageID.addSharedProperty(&_usEEP.p_sourceImageID);
        _usDenoiseilter.p_targetImageID.addSharedProperty(&_usDVR.p_sourceImageID);
        _usDenoiseilter.p_filterMode.selectById("gradientDiffusion");
        _usDenoiseilter.p_numberOfSteps.setValue(3);

        _usProxy.p_geometryID.setValue("us.proxy");
        _usProxy.p_geometryID.addSharedProperty(&_usEEP.p_geometryID);

        _usEEP.p_entryImageID.setValue("us.entry");
        _usEEP.p_entryImageID.addSharedProperty(&_usDVR.p_entryImageID);
        _usEEP.p_exitImageID.setValue("us.exit");
        _usEEP.p_exitImageID.addSharedProperty(&_usDVR.p_exitImageID);

        // TODO: replace this hardcoded domain by automatically determined from image min/max values
        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(128, tgt::vec2(0.f, 1.f));
        tf->addGeometry(TFGeometry1D::createQuad(tgt::vec2(0.f, 1.f), tgt::col4(0, 0, 0, 255), tgt::col4(255, 255, 255, 255)));
        _usFusion1.p_transferFunction.replaceTF(tf);
        _usFusion2.p_transferFunction.replaceTF(tf->clone());
        _usFusion3.p_transferFunction.replaceTF(tf->clone());
        _usFusion4.p_transferFunction.replaceTF(tf->clone());

        // TODO: replace this hardcoded domain by automatically determined from image min/max values
        Geometry1DTransferFunction* tf2 = new Geometry1DTransferFunction(256, tgt::vec2(0.f, 1.f));
        tf2->addGeometry(TFGeometry1D::createQuad(tgt::vec2(0.f, 1.f), tgt::col4(0, 0, 0, 255), tgt::col4(255, 255, 255, 255)));
        _usDVR.p_transferFunction.replaceTF(tf2);
        _usDVR.p_targetImageID.setValue("us.dvr");

        _renderTargetID.setValue("quadview.output");
    }

    void AdvancedUsVis::deinit() {
        _canvasSize.s_changed.disconnect(this);
        AutoEvaluationPipeline::deinit();
    }

    void AdvancedUsVis::execute() {
/*
        if (!_usReader.getInvalidationLevel().isValid()) {
            SimpleJobProc.enqueueJob(makeJob(this, &AdvancedUsVis::foobar));
        }
        if (!_usDenoiseilter.getInvalidationLevel().isValid()) {
            SimpleJobProc.enqueueJob(makeJob<AdvancedUsVis, AbstractProcessor*>(this, &AdvancedUsVis::executeProcessor, &_usDenoiseilter));
        }
        if (!_confidenceGenerator.getInvalidationLevel().isValid()) {
            SimpleJobProc.enqueueJob(makeJob<AdvancedUsVis, AbstractProcessor*>(this, &AdvancedUsVis::executeProcessor, &_confidenceGenerator));
        }


        for (std::vector<AbstractProcessor*>::iterator it = _processors.begin(); it != _processors.end(); ++it) {
            if (! (*it)->getInvalidationLevel().isValid())
                lockGLContextAndExecuteProcessor(*it);
        }*/
    }

    void AdvancedUsVis::keyEvent(tgt::KeyEvent* e) {
        if (e->pressed()) {
            switch (e->keyCode()) {
                case tgt::KeyEvent::K_UP:
                    _usFusion1.p_sliceNumber.increment();
                    break;
                case tgt::KeyEvent::K_DOWN:
                    _usFusion1.p_sliceNumber.decrement();
                    break;
            }
        }
    }

    void AdvancedUsVis::onProcessorValidated(AbstractProcessor* processor) {
        if (processor == &_usReader) {
            // convert data
            ScopedTypedData<ImageData> img(*_data, _usReader.p_targetImageID.getValue());
            if (img != 0) {
                _trackballEH->reinitializeCamera(img);
            }
        }
    }

}