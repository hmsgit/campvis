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

    NeuroDemo::NeuroDemo(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _lsp()
        , _tcp(&_canvasSize)
        , _ctReader()
        , _t1Reader()
        , _petReader()
        , _mvmpr(&_canvasSize)
        , _mvr(&_canvasSize)
        , _rtc(&_canvasSize)
    {
        _tcp.addLqModeProcessor(&_mvr);
        addEventListenerToBack(&_tcp);

        addProcessor(&_lsp);
        addProcessor(&_tcp);
        addProcessor(&_ctReader);
        addProcessor(&_t1Reader);
        addProcessor(&_petReader);
        addProcessor(&_mvmpr);
        addProcessor(&_mvr);
        addProcessor(&_rtc);
    }

    NeuroDemo::~NeuroDemo() {
    }

    void NeuroDemo::init() {
        AutoEvaluationPipeline::init();
        
        _tcp.p_image.setValue("ct.image");
        _renderTargetID.setValue("composed");

        _t1Reader.p_url.setValue(ShdrMgr.completePath("D:/Medical Data/K_Data/K_T1_bet04.GB306.am"));
        _t1Reader.p_targetImageID.setValue("t1_tf.image");
        _t1Reader.p_targetImageID.addSharedProperty(&_mvmpr.p_sourceImage1);
        _t1Reader.p_targetImageID.addSharedProperty(&_mvr.p_sourceImage1);
        _t1Reader.s_validated.connect(this, &NeuroDemo::onReaderValidated);

        _ctReader.p_url.setValue(ShdrMgr.completePath("D:/Medical Data/K_Data/K_CT_CoregT1.am"));
        _ctReader.p_targetImageID.setValue("ct.image");
        _ctReader.p_targetImageID.addSharedProperty(&_mvmpr.p_sourceImage2);
        _ctReader.p_targetImageID.addSharedProperty(&_mvr.p_sourceImage2);
        _ctReader.s_validated.connect(this, &NeuroDemo::onReaderValidated);

        _petReader.p_url.setValue(ShdrMgr.completePath("D:/Medical Data/K_Data/K_PET-CoregNMI_fl.am"));
        _petReader.p_targetImageID.setValue("pet.image");
        _petReader.p_targetImageID.addSharedProperty(&_mvmpr.p_sourceImage3);
        _petReader.p_targetImageID.addSharedProperty(&_mvr.p_sourceImage3);
        _petReader.s_validated.connect(this, &NeuroDemo::onReaderValidated);

        Geometry1DTransferFunction* t1_tf = new Geometry1DTransferFunction(128, cgt::vec2(0.f, .05f));
        t1_tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.25f, .65f), cgt::col4(170, 170, 128, 64), cgt::col4(192, 192, 128, 64)));
        _mvmpr.p_transferFunction1.replaceTF(t1_tf);
        _mvr.p_transferFunction1.replaceTF(t1_tf->clone());

        Geometry1DTransferFunction* ct_tf = new Geometry1DTransferFunction(128, cgt::vec2(0.f, .05f));
        ct_tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.8f, 1.f), cgt::col4(0, 150, 225, 192), cgt::col4(0, 192, 255, 255)));
        _mvmpr.p_transferFunction2.replaceTF(ct_tf);
        _mvr.p_transferFunction2.replaceTF(ct_tf->clone());

        Geometry1DTransferFunction* pet_tf = new Geometry1DTransferFunction(128, cgt::vec2(0.f, .05f));
        auto g = TFGeometry1D::createQuad(cgt::vec2(.8f, 1.0f), cgt::col4(255, 255, 0, 48), cgt::col4(255, 32, 192, 72));
        g->addKeyPoint(.9f, cgt::col4(255, 32, 0, 72));
        pet_tf->addGeometry(g);
        _mvmpr.p_transferFunction3.replaceTF(pet_tf);
        _mvr.p_transferFunction3.replaceTF(pet_tf->clone());

        _mvmpr.p_relativeToImageCenter.setValue(false);
        _mvmpr.p_use2DProjection.setValue(false);
        _mvmpr.p_planeSize.setValue(200.f);
        _mvmpr.p_outputImageId.setValue("result.mpr");
        _mvmpr.p_outputImageId.addSharedProperty(&_rtc.p_firstImageId);
        _mvmpr.p_outputImageId.addSharedProperty(&_mvr.p_geometryImageId);
        
        _mvr.p_outputImageId.setValue("result.rc");
        _mvr.p_outputImageId.addSharedProperty(&_rtc.p_secondImageId);
        _mvr.p_samplingRate.setValue(1.f);

        _rtc.p_compositingMethod.selectByOption(RenderTargetCompositor::CompositingModeDepth);
        _rtc.p_targetImageId.setValue("composed");
    }

    void NeuroDemo::onReaderValidated(AbstractProcessor* p) {
    }

    void NeuroDemo::onEvent(cgt::Event* e) {
        if (typeid(*e) == typeid(cgt::MouseEvent)) {
            cgt::MouseEvent* me = static_cast<cgt::MouseEvent*>(e);

            if (me->action() == cgt::MouseEvent::PRESSED && me->modifiers() & cgt::Event::CTRL) {
                // unproject mouse click through FirstHitPoint texture of raycaster
                ScopedTypedData<RenderData> rd(getDataContainer(), _mvr.p_outputImageId.getValue());
                if (rd != nullptr && rd->getNumColorTextures() == 3) {
                    const ImageRepresentationLocal* FHP = rd->getColorTexture(1)->getRepresentation<ImageRepresentationLocal>();
                    cgt::svec3 lookupPosition(me->x(), me->viewport().y - me->y(), 0);

                    if (cgt::hand(cgt::lessThan(lookupPosition, FHP->getSize()))) {
                        LINFO("Lookup Position: " << lookupPosition);

                        cgt::vec3 worldPosition(0.f);
                        for (size_t i = 0; i < 3; ++i)
                            worldPosition[i] = FHP->getElementNormalized(lookupPosition, i);

                        LINFO("World Position: " << worldPosition);

                        // add to base points
                        if (! (me->modifiers() & cgt::Event::SHIFT))
                            _mprBasePoints.clear();
                        _mprBasePoints.push_back(worldPosition);

                        if (_mprBasePoints.size() > 2) {
                            const cgt::vec3& a = _mprBasePoints[_mprBasePoints.size() - 3];
                            const cgt::vec3& b = _mprBasePoints[_mprBasePoints.size() - 2];
                            const cgt::vec3& c = _mprBasePoints[_mprBasePoints.size() - 1];

                            cgt::vec3 n = cgt::normalize(cgt::cross(b-a, c-a));
                            float d = cgt::dot(a, n);

                            _mvmpr.p_planeNormal.setValue(n);
                            _mvmpr.p_planeDistance.setValue(-d);
                        }
                    }
                }

                me->accept();
            }
        }

        //if (e->isAccepted())
            AutoEvaluationPipeline::onEvent(e);
    }

}