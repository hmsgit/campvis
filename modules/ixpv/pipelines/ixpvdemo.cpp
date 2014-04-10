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

#include "ixpvdemo.h"

#include "tgt/event/keyevent.h"
#include "core/datastructures/imagedata.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    IxpvDemo::IxpvDemo(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _xrayReader(&_canvasSize)
        , _ctReader()
        , _ctProxy()
        , _ctFullEEP(&_canvasSize)
        , _ctClippedEEP(&_canvasSize)
        , _ctDVR(&_canvasSize)
        , _ctFullDRR(&_canvasSize)
        , _ctClippedDRR(&_canvasSize)
        , _usReader()
        , _usSliceRenderer(&_canvasSize)
        , _compositor(&_canvasSize)
        , _compositor2(&_canvasSize)
        , _ixpvCompositor(&_canvasSize)
        , _camera("camera", "Camera")
        , _trackballHandler(0)
        , _wheelHandler(&_usSliceRenderer.p_sliceNumber)
    {
        addProcessor(&_xrayReader);

        addProcessor(&_usReader);
        addProcessor(&_usSliceRenderer);

        addProcessor(&_ctReader);
        addProcessor(&_ctProxy);
        addProcessor(&_ctFullEEP);
        addProcessor(&_ctClippedEEP);
        addProcessor(&_ctDVR);
        addProcessor(&_ctFullDRR);
        addProcessor(&_ctClippedDRR);

        addProcessor(&_compositor);
        addProcessor(&_compositor2);
        addProcessor(&_ixpvCompositor);

        addProperty(_camera);

        _trackballHandler = new TrackballNavigationEventListener(&_camera, &_canvasSize);
        _trackballHandler->addLqModeProcessor(&_ctDVR);
        _trackballHandler->addLqModeProcessor(&_ctFullDRR);
        _trackballHandler->addLqModeProcessor(&_ctClippedDRR);
        addEventListenerToBack(&_wheelHandler);
        addEventListenerToBack(_trackballHandler);
    }

    IxpvDemo::~IxpvDemo() {
        delete _trackballHandler;
    }

    void IxpvDemo::init() {
        AutoEvaluationPipeline::init();

        // = Camera Setup =================================================================================
        _camera.addSharedProperty(&_ctFullEEP.p_camera);
        _camera.addSharedProperty(&_ctClippedEEP.p_camera);
        _camera.addSharedProperty(&_ctDVR.p_camera);
        _camera.addSharedProperty(&_ctFullDRR.p_camera);
        _camera.addSharedProperty(&_ctClippedDRR.p_camera);
        _camera.addSharedProperty(&_usSliceRenderer.p_camera);

        // = X-Ray Setup ==================================================================================
        _xrayReader.p_url.setValue("D:\\Medical Data\\XrayDepthPerception\\DataCowLeg\\Cowleg_CarmXrayImages\\APView_1.jpg");
        _xrayReader.p_targetImageID.setValue("xray.image");

        // = CT Setup =====================================================================================

        _ctReader.p_targetImageID.addSharedProperty(&_ctProxy.p_sourceImageID);
        _ctReader.p_targetImageID.addSharedProperty(&_ctFullEEP.p_sourceImageID);
        _ctReader.p_targetImageID.addSharedProperty(&_ctClippedEEP.p_sourceImageID);
        _ctReader.p_targetImageID.addSharedProperty(&_ctDVR.p_sourceImageID);
        _ctReader.p_targetImageID.addSharedProperty(&_ctFullDRR.p_sourceImageID);
        _ctReader.p_targetImageID.addSharedProperty(&_ctClippedDRR.p_sourceImageID);
        _ctReader.p_url.setValue("D:/Medical Data/XrayDepthPerception/DataCowLeg/Cowleg_CT_8007_003_5mm_STD_20110224/phantom.mhd");
        _ctReader.p_targetImageID.setValue("ct.image");
        _ctReader.s_validated.connect(this, &IxpvDemo::onProcessorValidated);

        _ctProxy.p_geometryID.setValue("ct.proxy");
        _ctProxy.p_geometryID.addSharedProperty(&_ctFullEEP.p_geometryID);
        _ctProxy.p_geometryID.addSharedProperty(&_ctClippedEEP.p_geometryID);

        _ctFullEEP.p_entryImageID.setValue("ct.entry.full");
        _ctFullEEP.p_entryImageID.addSharedProperty(&_ctFullDRR.p_entryImageID);
        _ctFullEEP.p_entryImageID.addSharedProperty(&_ctDVR.p_entryImageID);
        _ctFullEEP.p_exitImageID.setValue("ct.exit.full");
        _ctFullEEP.p_exitImageID.addSharedProperty(&_ctFullDRR.p_exitImageID);
        _ctFullEEP.p_exitImageID.addSharedProperty(&_ctDVR.p_exitImageID);

        _ctClippedEEP.p_entryImageID.setValue("ct.entry.clipped");
        _ctClippedEEP.p_entryImageID.addSharedProperty(&_ctClippedDRR.p_entryImageID);
        _ctClippedEEP.p_exitImageID.setValue("ct.exit.clipped");
        _ctClippedEEP.p_exitImageID.addSharedProperty(&_ctClippedDRR.p_exitImageID);

        Geometry1DTransferFunction* tfDvr = new Geometry1DTransferFunction(128, tgt::vec2(0.f, .08f));
        //        tf->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.5f, 1.f), tgt::col4(0, 0, 0, 0), tgt::col4(0, 0, 0, 180)));
        tfDvr->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.4f, .6f), tgt::col4(255, 192, 0, 255), tgt::col4(255, 192, 0, 255)));
        _ctDVR.p_transferFunction.replaceTF(tfDvr);
        _ctDVR.p_targetImageID.setValue("ct.dvr");
        _ctDVR.p_samplingRate.setValue(1.f);

        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(128, tgt::vec2(0.f, .08f));
        tf->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.5f, 1.f), tgt::col4(0, 0, 0, 0), tgt::col4(0, 0, 0, 180)));
        _ctFullDRR.p_transferFunction.replaceTF(tf);
        _ctFullDRR.p_targetImageID.setValue("ct.drr.full");
        _ctFullDRR.p_samplingRate.setValue(1.f);
        _ctFullDRR.p_invertMapping.setValue(true);
        
        Geometry1DTransferFunction* tf2 = new Geometry1DTransferFunction(128, tgt::vec2(0.f, .08f));
        tf2->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.5f, 1.f), tgt::col4(0, 0, 0, 0), tgt::col4(0, 0, 0, 180)));
        _ctClippedDRR.p_transferFunction.replaceTF(tf2);
        _ctClippedDRR.p_targetImageID.setValue("ct.drr.clipped");
        _ctClippedDRR.p_samplingRate.setValue(1.f);
        _ctClippedDRR.p_invertMapping.setValue(true);

        // = US Setup =====================================================================================

        _usReader.s_validated.connect(this, &IxpvDemo::onProcessorValidated);
        _usReader.p_url.setValue("D:\\Medical Data\\XrayDepthPerception\\DataCowLeg\\Ultrasound\\gaussianSmoothedUS_UChar.mhd");
        _usReader.p_targetImageID.setValue("us.image");
        _usReader.p_targetImageID.addSharedProperty(&_usSliceRenderer.p_sourceImageID);
        _usReader.p_imageOffset.setValue(tgt::vec3(-80.f, 0.f, -80.f));
        _usReader.p_voxelSize.setValue(tgt::vec3(1.f, 1.f, 1.3f));

        _usSliceRenderer.p_targetImageID.setValue("us.slice");
        _usSliceRenderer.p_targetImageID.addSharedProperty(&_ctClippedEEP.p_geometryImageId);
        
        _usSliceRenderer.p_sliceNumber.setValue(0);


        // = Compositing Setup ============================================================================

        _xrayReader.p_targetImageID.addSharedProperty(&_compositor.p_firstImageId);
        _ctFullDRR.p_targetImageID.addSharedProperty(&_compositor.p_secondImageId);
        _compositor.p_targetImageId.setValue("composed");
        _compositor.p_compositingMethod.selectById("diff");

        _ctDVR.p_targetImageID.addSharedProperty(&_compositor2.p_firstImageId);
        _usSliceRenderer.p_targetImageID.addSharedProperty(&_compositor2.p_secondImageId);
        _compositor2.p_targetImageId.setValue("composed2");
        _compositor2.p_compositingMethod.selectById("depth");

        _xrayReader.p_targetImageID.addSharedProperty(&_ixpvCompositor.p_xRayImageId);
        _usSliceRenderer.p_targetImageID.addSharedProperty(&_ixpvCompositor.p_3dSliceImageId);
        _ctFullDRR.p_targetImageID.addSharedProperty(&_ixpvCompositor.p_drrFullImageId);
        _ctClippedDRR.p_targetImageID.addSharedProperty(&_ixpvCompositor.p_drrClippedImageId);
        _ixpvCompositor.p_targetImageId.setValue("ixpv");

        _renderTargetID.setValue("ixpv");
    }

    void IxpvDemo::onProcessorValidated(AbstractProcessor* processor) {
        if (processor == &_ctReader) {
            ImageRepresentationLocal::ScopedRepresentation local(*_data, _ctReader.p_targetImageID.getValue());
            if (local != 0) {
                // set TF handles
                Interval<float> ii = local->getNormalizedIntensityRange();
                _ctDVR.p_transferFunction.setImageHandle(local.getDataHandle());
                _ctDVR.p_transferFunction.getTF()->setIntensityDomain(tgt::vec2(ii.getLeft(), ii.getRight()));
                _ctFullDRR.p_transferFunction.setImageHandle(local.getDataHandle());
                _ctFullDRR.p_transferFunction.getTF()->setIntensityDomain(tgt::vec2(.3f, .73f));
                _ctClippedDRR.p_transferFunction.setImageHandle(local.getDataHandle());
                _ctClippedDRR.p_transferFunction.getTF()->setIntensityDomain(tgt::vec2(.3f, .73f));

                // update camera
                tgt::Bounds volumeExtent = local->getParent()->getWorldBounds();

                _trackballHandler->setSceneBounds(volumeExtent);
                _trackballHandler->setCenter(volumeExtent.center());
                _trackballHandler->reinitializeCamera(
                    tgt::vec3(17.6188f, -386.82f, 69.0831f), 
                    tgt::vec3(-40.0402f, 0.1685f, 27.9503f), 
                    tgt::vec3(0.9882f, 0.1504f, 0.0299f));
            }   
        }
        else if (processor == &_usReader) {
            // set TF handles
            ImageRepresentationLocal::ScopedRepresentation local(*_data, _usReader.p_targetImageID.getValue());
            if (local != 0) {
                Interval<float> ii = local->getNormalizedIntensityRange();
                _usSliceRenderer.p_transferFunction.getTF()->setIntensityDomain(tgt::vec2(ii.getLeft(), ii.getRight()));
                _usSliceRenderer.p_sliceNumber.setValue(125);
            }
        }
    }

}
