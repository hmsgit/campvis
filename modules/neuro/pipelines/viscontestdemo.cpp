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

#include "viscontestdemo.h"

#include "cgt/event/keyevent.h"
#include "cgt/event/mouseevent.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationlocal.h"
#include "core/datastructures/imageseries.h"
#include "core/datastructures/renderdata.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    ViscontestDemo::ViscontestDemo(DataContainer& dc)
        : AutoEvaluationPipeline(dc, getId())
        , _lsp()
        , _tcp(&_canvasSize)
        , _fiberReader()
        , _fiberRenderer(&_canvasSize)
        , _t1PostReader()
        , _t1PreReader()
        , _flairReader()
        , _mvmpr2D(&_canvasSize)
        , _mvmpr3D(&_canvasSize)
        , _mvr(&_canvasSize)
        , _rtc1(&_canvasSize)
        , _rtc2(&_canvasSize)
        , _horizontalSplitter(2, ViewportSplitter::HORIZONTAL, &_canvasSize)
        , _slicePositionEventHandler(&_mvmpr2D.p_planeDistance)
    {
        _tcp.addLqModeProcessor(&_mvr);
        addEventListenerToBack(&_horizontalSplitter);

        addProcessor(&_lsp);
        addProcessor(&_tcp);
        addProcessor(&_fiberReader);
        addProcessor(&_fiberRenderer);
        addProcessor(&_t1PostReader);
        addProcessor(&_t1PreReader);
        addProcessor(&_flairReader);
        addProcessor(&_mvmpr2D);
        addProcessor(&_mvmpr3D);
        addProcessor(&_mvr);
        addProcessor(&_rtc1);
        addProcessor(&_rtc2);

        _horizontalSplitter.p_outputImageId.setValue("ViscontestDemo");
        _horizontalSplitter.setInputImageIdProperty(0, &_mvmpr2D.p_outputImageId);
        _horizontalSplitter.setInputImageIdProperty(1, &_rtc2.p_targetImageId);

        _tcp.setViewportSizeProperty(&_horizontalSplitter.p_subViewViewportSize);
        _mvmpr2D.setViewportSizeProperty(&_horizontalSplitter.p_subViewViewportSize);
        _mvmpr3D.setViewportSizeProperty(&_horizontalSplitter.p_subViewViewportSize);
        _mvr.setViewportSizeProperty(&_horizontalSplitter.p_subViewViewportSize);
        _rtc1.setViewportSizeProperty(&_horizontalSplitter.p_subViewViewportSize);
        _rtc2.setViewportSizeProperty(&_horizontalSplitter.p_subViewViewportSize);
    }

    ViscontestDemo::~ViscontestDemo() {
    }

    void ViscontestDemo::init() {
        AutoEvaluationPipeline::init();
        _horizontalSplitter.init();
        _horizontalSplitter.s_onEvent.connect(this, &ViscontestDemo::onSplitterEvent);
        
        _tcp.p_image.setValue("ct.image");
        _renderTargetID.setValue("ViscontestDemo");

        _fiberReader.p_url.setValue(ShdrMgr.completePath("modules/neuro/sampledata/case1/tumor.trk"));
        _fiberReader.p_scaling.setValue(cgt::vec3(100.f));
        _fiberReader.p_outputId.setValue("fibers");
        _fiberReader.p_outputId.addSharedProperty(&_fiberRenderer.p_strainId);

        _fiberRenderer.p_renderTargetID.setValue("fibres.rendered");
        _fiberRenderer.p_renderTargetID.addSharedProperty(&_mvr.p_geometryImageId);
        _fiberRenderer.p_renderTargetID.addSharedProperty(&_rtc1.p_firstImageId);

        _t1PostReader.p_url.setValue(ShdrMgr.completePath("modules/neuro/sampledata/case1/case1_T1_pre.mhd"));
        _t1PostReader.p_targetImageID.setValue("t1_pre.image");
        _t1PostReader.p_targetImageID.addSharedProperty(&_mvmpr2D.p_sourceImage1);
        _t1PostReader.p_targetImageID.addSharedProperty(&_mvmpr3D.p_sourceImage1);
        _t1PostReader.p_targetImageID.addSharedProperty(&_mvr.p_sourceImage1);
        _t1PostReader.s_validated.connect(this, &ViscontestDemo::onReaderValidated);

        _t1PreReader.p_url.setValue(ShdrMgr.completePath("modules/neuro/sampledata/case1/case1_T1_post.mhd"));
        _t1PreReader.p_targetImageID.setValue("t1_post.image");
        _t1PreReader.p_targetImageID.addSharedProperty(&_mvmpr2D.p_sourceImage2);
        _t1PreReader.p_targetImageID.addSharedProperty(&_mvmpr3D.p_sourceImage2);
        _t1PreReader.p_targetImageID.addSharedProperty(&_mvr.p_sourceImage2);
        _t1PreReader.s_validated.connect(this, &ViscontestDemo::onReaderValidated);

        _flairReader.p_url.setValue(ShdrMgr.completePath("modules/neuro/sampledata/case1/case1_FLAIR.mhd"));
        _flairReader.p_targetImageID.setValue("flair.image");
        _flairReader.p_targetImageID.addSharedProperty(&_mvmpr2D.p_sourceImage3);
        _flairReader.p_targetImageID.addSharedProperty(&_mvmpr3D.p_sourceImage3);
        _flairReader.p_targetImageID.addSharedProperty(&_mvr.p_sourceImage3);
        _flairReader.s_validated.connect(this, &ViscontestDemo::onReaderValidated);

        Geometry1DTransferFunction* t1_tf_rc = new Geometry1DTransferFunction(128, cgt::vec2(0.f, .023f));
        t1_tf_rc->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.06f, .11f), cgt::col4(57, 57, 57, 32), cgt::col4(196, 196, 196, 16)));
        _mvr.p_transferFunction1.replaceTF(t1_tf_rc);

        Geometry1DTransferFunction* t1_tf_mpr = new Geometry1DTransferFunction(128, cgt::vec2(0.f, .023f));
        t1_tf_mpr->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.06f, .4f), cgt::col4(57, 57, 57, 32), cgt::col4(196, 196, 196, 16)));
        _mvmpr2D.p_transferFunction1.replaceTF(t1_tf_mpr);
        _mvmpr3D.p_transferFunction1.replaceTF(t1_tf_mpr->clone());

        Geometry1DTransferFunction* ct_tf = new Geometry1DTransferFunction(128, cgt::vec2(0.f, .0421f));
        ct_tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.381f, .779f), cgt::col4(0, 100, 150, 128), cgt::col4(0, 192, 255, 172)));
        _mvmpr2D.p_transferFunction2.replaceTF(ct_tf);
        _mvmpr3D.p_transferFunction2.replaceTF(ct_tf->clone());
        _mvr.p_transferFunction2.replaceTF(ct_tf->clone());

        Geometry1DTransferFunction* flair_tf = new Geometry1DTransferFunction(128, cgt::vec2(0.f, .0193027f));
        auto g = TFGeometry1D::createQuad(cgt::vec2(0.34f, 0.42f), cgt::col4(255, 255, 0, 80), cgt::col4(255, 32, 192, 128));
        g->addKeyPoint(.9f, cgt::col4(255, 32, 192, 150));
        flair_tf->addGeometry(g);
        _mvmpr2D.p_transferFunction3.replaceTF(flair_tf);
        _mvmpr3D.p_transferFunction3.replaceTF(flair_tf->clone());
        _mvr.p_transferFunction3.replaceTF(flair_tf->clone());

        _mvmpr2D.p_relativeToImageCenter.setValue(false);
        _mvmpr2D.p_use2DProjection.setValue(true);
        _mvmpr2D.p_planeSize.setValue(200.f);
        _mvmpr2D.p_showWireframe.setValue(false);
        _mvmpr2D.p_transparency.setValue(0.f);
        _mvmpr2D.p_outputImageId.setValue("result.mpr.2d");

        _mvmpr2D.p_planeSize.addSharedProperty(&_mvmpr3D.p_planeSize);
        _mvmpr2D.p_planeDistance.addSharedProperty(&_mvmpr3D.p_planeDistance);
        _mvmpr2D.p_planeNormal.addSharedProperty(&_mvmpr3D.p_planeNormal);

        _mvmpr3D.p_relativeToImageCenter.setValue(false);
        _mvmpr3D.p_use2DProjection.setValue(false);
        _mvmpr3D.p_outputImageId.setValue("result.mpr.3d");
        _mvmpr3D.p_showWireframe.setValue(true);
        _mvmpr3D.p_transparency.setValue(0.5f);
        _mvmpr3D.p_outputImageId.addSharedProperty(&_rtc2.p_firstImageId);
        
        _mvr.p_outputImageId.setValue("result.rc");
        _mvr.p_outputImageId.addSharedProperty(&_rtc1.p_secondImageId);
        _mvr.p_samplingRate.setValue(1.f);

        _rtc1.p_compositingMethod.selectByOption(RenderTargetCompositor::CompositingModeDepth);
        _rtc1.p_targetImageId.setValue("composed1");
        _rtc1.p_targetImageId.addSharedProperty(&_rtc2.p_secondImageId);

        _rtc2.p_compositingMethod.selectByOption(RenderTargetCompositor::CompositingModeDepth);
        _rtc2.p_targetImageId.setValue("composed");
    }

    void ViscontestDemo::deinit() {
        _horizontalSplitter.s_onEvent.disconnect(this);
        _horizontalSplitter.deinit();
        AutoEvaluationPipeline::deinit();
    }

    void ViscontestDemo::executePipeline() {
        AutoEvaluationPipeline::executePipeline();
        _horizontalSplitter.render(getDataContainer());
    }

    void ViscontestDemo::onReaderValidated(AbstractProcessor* p) {
    }

    void ViscontestDemo::onSplitterEvent(size_t index, cgt::Event* e) {
        if (typeid(*e) == typeid(cgt::MouseEvent)) {
            if (index == 0) {
                _slicePositionEventHandler.onEvent(e);
            }
            else if (index == 1) {
                _tcp.onEvent(e);
            }
        }
    }
    
}