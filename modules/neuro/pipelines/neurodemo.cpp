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

#include "neurodemo.h"

#include "cgt/event/keyevent.h"
#include "cgt/event/mouseevent.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationlocal.h"
#include "core/datastructures/imageseries.h"
#include "core/datastructures/renderdata.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    NeuroDemo::NeuroDemo(DataContainer& dc)
        : AutoEvaluationPipeline(dc, getId())
        , _lsp()
        , _tcp(&_canvasSize)
        , _ctReader()
        , _t1Reader()
        , _petReader()
        , _mvmpr2D(&_canvasSize)
        , _mvmpr3D(&_canvasSize)
        , _mvr(&_canvasSize)
        , _rtc(&_canvasSize)
        , _horizontalSplitter(2, ViewportSplitter::HORIZONTAL, &_canvasSize)
        , _slicePositionEventHandler(&_mvmpr2D.p_planeDistance)
    {
        _tcp.addLqModeProcessor(&_mvr);
        addEventListenerToBack(&_horizontalSplitter);

        addProcessor(&_lsp);
        addProcessor(&_tcp);
        addProcessor(&_ctReader);
        addProcessor(&_t1Reader);
        addProcessor(&_petReader);
        addProcessor(&_mvmpr2D);
        addProcessor(&_mvmpr3D);
        addProcessor(&_mvr);
        addProcessor(&_rtc);

        _horizontalSplitter.p_outputImageId.setValue("NeuroDemo");
        _horizontalSplitter.setInputImageIdProperty(0, &_mvmpr2D.p_outputImageId);
        _horizontalSplitter.setInputImageIdProperty(1, &_rtc.p_targetImageId);

        _tcp.setViewportSizeProperty(&_horizontalSplitter.p_subViewViewportSize);
        _mvmpr2D.setViewportSizeProperty(&_horizontalSplitter.p_subViewViewportSize);
        _mvmpr3D.setViewportSizeProperty(&_horizontalSplitter.p_subViewViewportSize);
        _mvr.setViewportSizeProperty(&_horizontalSplitter.p_subViewViewportSize);
        _rtc.setViewportSizeProperty(&_horizontalSplitter.p_subViewViewportSize);
    }

    NeuroDemo::~NeuroDemo() {
    }

    void NeuroDemo::init() {
        AutoEvaluationPipeline::init();
        _horizontalSplitter.init();
        _horizontalSplitter.s_onEvent.connect(this, &NeuroDemo::onSplitterEvent);
        
        _tcp.p_image.setValue("ct.image");
        _renderTargetID.setValue("NeuroDemo");

        _t1Reader.p_url.setValue(ShdrMgr.completePath("D:/Medical Data/K_Data/K_T1_bet04.GB306.am"));
        _t1Reader.p_targetImageID.setValue("t1_tf.image");
        _t1Reader.p_targetImageID.addSharedProperty(&_mvmpr2D.p_sourceImage1);
        _t1Reader.p_targetImageID.addSharedProperty(&_mvmpr3D.p_sourceImage1);
        _t1Reader.p_targetImageID.addSharedProperty(&_mvr.p_sourceImage1);
        _t1Reader.s_validated.connect(this, &NeuroDemo::onReaderValidated);

        _ctReader.p_url.setValue(ShdrMgr.completePath("D:/Medical Data/K_Data/K_CT_CoregT1.am"));
        _ctReader.p_targetImageID.setValue("ct.image");
        _ctReader.p_targetImageID.addSharedProperty(&_mvmpr2D.p_sourceImage2);
        _ctReader.p_targetImageID.addSharedProperty(&_mvmpr3D.p_sourceImage2);
        _ctReader.p_targetImageID.addSharedProperty(&_mvr.p_sourceImage2);
        _ctReader.s_validated.connect(this, &NeuroDemo::onReaderValidated);

        _petReader.p_url.setValue(ShdrMgr.completePath("D:/Medical Data/K_Data/K_PET-CoregNMI_fl.am"));
        _petReader.p_targetImageID.setValue("pet.image");
        _petReader.p_targetImageID.addSharedProperty(&_mvmpr2D.p_sourceImage3);
        _petReader.p_targetImageID.addSharedProperty(&_mvmpr3D.p_sourceImage3);
        _petReader.p_targetImageID.addSharedProperty(&_mvr.p_sourceImage3);
        _petReader.s_validated.connect(this, &NeuroDemo::onReaderValidated);

        Geometry1DTransferFunction* t1_tf = new Geometry1DTransferFunction(128, cgt::vec2(0.f, .05f));
        t1_tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.25f, .65f), cgt::col4(170, 170, 128, 64), cgt::col4(192, 192, 128, 64)));
        _mvmpr2D.p_transferFunction1.replaceTF(t1_tf);
        _mvmpr3D.p_transferFunction1.replaceTF(t1_tf->clone());
        _mvr.p_transferFunction1.replaceTF(t1_tf->clone());

        Geometry1DTransferFunction* ct_tf = new Geometry1DTransferFunction(128, cgt::vec2(0.f, .05f));
        ct_tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.8f, 1.f), cgt::col4(0, 150, 225, 192), cgt::col4(0, 192, 255, 255)));
        _mvmpr2D.p_transferFunction2.replaceTF(ct_tf);
        _mvmpr3D.p_transferFunction2.replaceTF(ct_tf->clone());
        _mvr.p_transferFunction2.replaceTF(ct_tf->clone());

        Geometry1DTransferFunction* pet_tf = new Geometry1DTransferFunction(128, cgt::vec2(0.f, .05f));
        auto g = TFGeometry1D::createQuad(cgt::vec2(.8f, 1.0f), cgt::col4(255, 255, 0, 48), cgt::col4(255, 32, 192, 72));
        g->addKeyPoint(.9f, cgt::col4(255, 32, 0, 72));
        pet_tf->addGeometry(g);
        _mvmpr2D.p_transferFunction3.replaceTF(pet_tf);
        _mvmpr3D.p_transferFunction3.replaceTF(pet_tf->clone());
        _mvr.p_transferFunction3.replaceTF(pet_tf->clone());

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
        _mvmpr3D.p_outputImageId.addSharedProperty(&_rtc.p_firstImageId);
        //_mvmpr3D.p_outputImageId.addSharedProperty(&_mvr.p_geometryImageId);
        
        _mvr.p_outputImageId.setValue("result.rc");
        _mvr.p_outputImageId.addSharedProperty(&_rtc.p_secondImageId);
        _mvr.p_samplingRate.setValue(1.f);

        _rtc.p_compositingMethod.selectByOption(RenderTargetCompositor::CompositingModeDepth);
        _rtc.p_targetImageId.setValue("composed");
    }

    void NeuroDemo::deinit() {
        _horizontalSplitter.s_onEvent.disconnect(this);
        _horizontalSplitter.deinit();
        AutoEvaluationPipeline::deinit();
    }

    void NeuroDemo::executePipeline() {
        AutoEvaluationPipeline::executePipeline();
        _horizontalSplitter.render(getDataContainer());
    }

    void NeuroDemo::onReaderValidated(AbstractProcessor* p) {
    }

    void NeuroDemo::onSplitterEvent(size_t index, cgt::Event* e) {

        if (typeid(*e) == typeid(cgt::MouseEvent)) {
            cgt::MouseEvent* me = static_cast<cgt::MouseEvent*>(e);

            if (index == 0) {
                if (me->action() == cgt::MouseEvent::PRESSED && me->modifiers() & cgt::Event::CTRL) {
                    // unproject mouse click through FirstHitPoint texture of raycaster
                    ScopedTypedData<RenderData> rd(getDataContainer(), _mvmpr2D.p_outputImageId.getValue());
                    if (rd != nullptr && rd->getNumColorTextures() >= 2) {
                        const ImageRepresentationLocal* FHP = rd->getColorTexture(1)->getRepresentation<ImageRepresentationLocal>();
                        cgt::svec3 lookupPosition(me->x(), me->viewport().y - me->y(), 0);

                        if (cgt::hand(cgt::lessThan(lookupPosition, FHP->getSize()))) {
                            LINFO("Lookup Position: " << lookupPosition);

                            cgt::vec3 worldPosition(0.f);
                            for (size_t i = 0; i < 3; ++i)
                                worldPosition[i] = FHP->getElementNormalized(lookupPosition, i);

                            addBasePoint(!(me->modifiers() & cgt::Event::SHIFT), worldPosition);
                        }
                    }
                }
                else {
                    _slicePositionEventHandler.onEvent(e);
                }
            }
            else if (index == 1) {
                if (me->action() == cgt::MouseEvent::PRESSED && me->modifiers() & cgt::Event::CTRL) {
                    // unproject mouse click through FirstHitPoint texture of raycaster
                    ScopedTypedData<RenderData> rd(getDataContainer(), _mvr.p_outputImageId.getValue());
                    if (rd != nullptr && rd->getNumColorTextures() >= 2) {
                        const ImageRepresentationLocal* FHP = rd->getColorTexture(1)->getRepresentation<ImageRepresentationLocal>();
                        cgt::svec3 lookupPosition(me->x(), me->viewport().y - me->y(), 0);

                        if (cgt::hand(cgt::lessThan(lookupPosition, FHP->getSize()))) {
                            LINFO("Lookup Position: " << lookupPosition);

                            cgt::vec3 worldPosition(0.f);
                            for (size_t i = 0; i < 3; ++i)
                                worldPosition[i] = FHP->getElementNormalized(lookupPosition, i);

                            addBasePoint(!(me->modifiers() & cgt::Event::SHIFT), worldPosition);
                        }
                    }
                }
                else {
                    _tcp.onEvent(e);
                }
            }
        }
    }

    void NeuroDemo::addBasePoint(bool clear, const cgt::vec3& position) {
        LINFO("World Position: " << position);

        // add to base points
        if (clear)
            _mprBasePoints.clear();
        _mprBasePoints.push_back(position);

        if (_mprBasePoints.size() > 2) {
            const cgt::vec3& a = _mprBasePoints[_mprBasePoints.size() - 3];
            const cgt::vec3& b = _mprBasePoints[_mprBasePoints.size() - 2];
            const cgt::vec3& c = _mprBasePoints[_mprBasePoints.size() - 1];

            cgt::vec3 n = cgt::normalize(cgt::cross(b-a, c-a));
            float d = cgt::dot(a, n);

            _mvmpr2D.p_planeNormal.setValue(n);
            _mvmpr2D.p_planeDistance.setValue(-d);
        }
    }

}