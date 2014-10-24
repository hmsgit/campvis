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

#include "ixpvdemo.h"

#include "cgt/event/keyevent.h"
#include "core/datastructures/imagedata.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    IxpvDemo::IxpvDemo(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _xrayReader()
        , _ctReader()
        , _vrFull(&_canvasSize, new DRRRaycaster(&_canvasSize))
        , _vrClipped(&_canvasSize, new DRRRaycaster(&_canvasSize))
        , _usReader()
        , _usSliceRenderer(&_canvasSize)
        , _compositor(&_canvasSize)
        , _ixpvCompositor(&_canvasSize)
        , _camera("camera", "Camera")
        , _trackballHandler(0)
        , _wheelHandler(&_usSliceRenderer.p_sliceNumber)
    {
        addProcessor(&_xrayReader);

        addProcessor(&_usReader);
        addProcessor(&_usSliceRenderer);

        addProcessor(&_ctReader);
        addProcessor(&_vrFull);
        addProcessor(&_vrClipped);

        addProcessor(&_compositor);
        addProcessor(&_ixpvCompositor);

        addProperty(_camera);

        _trackballHandler = new TrackballNavigationEventListener(&_camera, &_canvasSize);
        _trackballHandler->addLqModeProcessor(&_vrFull);
        _trackballHandler->addLqModeProcessor(&_vrClipped);
        addEventListenerToBack(&_wheelHandler);
//        addEventListenerToBack(_trackballHandler);
    }

    IxpvDemo::~IxpvDemo() {
        delete _trackballHandler;
    }

    void IxpvDemo::init() {
        AutoEvaluationPipeline::init();

        // = Camera Setup =================================================================================
        _camera.addSharedProperty(&_vrFull.p_camera);
        _camera.addSharedProperty(&_vrClipped.p_camera);
        _camera.addSharedProperty(&_usSliceRenderer.p_camera);

        // = X-Ray Setup ==================================================================================
        _xrayReader.p_url.setValue("D:\\Medical Data\\XrayDepthPerception\\DataCowLeg\\Cowleg_CarmXrayImages\\APView_1.jpg");
        _xrayReader.p_targetImageID.setValue("xray.image");

        // = CT Setup =====================================================================================

        _ctReader.p_targetImageID.addSharedProperty(&_vrFull.p_inputVolume);
        _ctReader.p_targetImageID.addSharedProperty(&_vrClipped.p_inputVolume);
        _ctReader.p_url.setValue("D:/Medical Data/XrayDepthPerception/DataCowLeg/Cowleg_CT_8007_003_5mm_STD_20110224/phantom.mhd");
        _ctReader.p_targetImageID.setValue("ct.image");
        _ctReader.s_validated.connect(this, &IxpvDemo::onProcessorValidated);

        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(128, cgt::vec2(0.f, .08f));
        tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.5f, 1.f), cgt::col4(0, 0, 0, 0), cgt::col4(0, 0, 0, 180)));
        static_cast<TransferFunctionProperty*>(_vrFull.getNestedProperty("RaycasterProps::TransferFunction"))->replaceTF(tf);
        _vrFull.p_outputImage.setValue("ct.drr.full");
        static_cast<FloatProperty*>(_vrFull.getNestedProperty("RaycasterProps::SamplingRate"))->setValue(1.f);
        static_cast<BoolProperty*>(_vrFull.getNestedProperty("RaycasterProps::InvertMapping"))->setValue(true);
        
        Geometry1DTransferFunction* tf2 = new Geometry1DTransferFunction(128, cgt::vec2(0.f, .08f));
        tf2->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.5f, 1.f), cgt::col4(0, 0, 0, 0), cgt::col4(0, 0, 0, 180)));
        static_cast<TransferFunctionProperty*>(_vrClipped.getNestedProperty("RaycasterProps::TransferFunction"))->replaceTF(tf2);
        _vrClipped.p_outputImage.setValue("ct.drr.clipped");
        static_cast<FloatProperty*>(_vrClipped.getNestedProperty("RaycasterProps::SamplingRate"))->setValue(1.f);
        static_cast<BoolProperty*>(_vrClipped.getNestedProperty("RaycasterProps::InvertMapping"))->setValue(true);

        // = US Setup =====================================================================================

        _usReader.s_validated.connect(this, &IxpvDemo::onProcessorValidated);
        _usReader.p_url.setValue("D:\\Medical Data\\XrayDepthPerception\\DataCowLeg\\Ultrasound\\gaussianSmoothedUS_UChar.mhd");
        _usReader.p_targetImageID.setValue("us.image");
        _usReader.p_targetImageID.addSharedProperty(&_usSliceRenderer.p_sourceImageID);
        _usReader.p_imageOffset.setValue(cgt::vec3(-80.f, 0.f, -80.f));
        _usReader.p_voxelSize.setValue(cgt::vec3(1.f, 1.f, 1.3f));

        _usSliceRenderer.p_targetImageID.setValue("us.slice");
        _usSliceRenderer.p_targetImageID.addSharedProperty(_vrClipped.getNestedProperty("EEPProps::GeometryImageId"));
        
        _usSliceRenderer.p_sliceNumber.setValue(0);


        // = Compositing Setup ============================================================================

        _xrayReader.p_targetImageID.addSharedProperty(&_compositor.p_firstImageId);
        _vrFull.p_outputImage.addSharedProperty(&_compositor.p_secondImageId);
        _compositor.p_targetImageId.setValue("composed");
        _compositor.p_compositingMethod.selectById("diff");

        _xrayReader.p_targetImageID.addSharedProperty(&_ixpvCompositor.p_xRayImageId);
        _usSliceRenderer.p_targetImageID.addSharedProperty(&_ixpvCompositor.p_3dSliceImageId);
        _vrFull.p_outputImage.addSharedProperty(&_ixpvCompositor.p_drrFullImageId);
        _vrClipped.p_outputImage.addSharedProperty(&_ixpvCompositor.p_drrClippedImageId);
        _ixpvCompositor.p_targetImageId.setValue("ixpv");

        _renderTargetID.setValue("ixpv");
    }

    void IxpvDemo::onProcessorValidated(AbstractProcessor* processor) {
        if (processor == &_ctReader) {
            ImageRepresentationLocal::ScopedRepresentation local(*_data, _ctReader.p_targetImageID.getValue());
            if (local != 0) {
                // update camera
                cgt::Bounds volumeExtent = local->getParent()->getWorldBounds();

                _trackballHandler->setSceneBounds(volumeExtent);
                _trackballHandler->setCenter(volumeExtent.center());
                _trackballHandler->reinitializeCamera(
                    cgt::vec3(17.6188f, -386.82f, 69.0831f), 
                    cgt::vec3(-40.0402f, 0.1685f, 27.9503f), 
                    cgt::vec3(0.9882f, 0.1504f, 0.0299f));
            }   
        }
        else if (processor == &_usReader) {
            // set TF handles
            ImageRepresentationLocal::ScopedRepresentation local(*_data, _usReader.p_targetImageID.getValue());
            if (local != 0) {
                Interval<float> ii = local->getNormalizedIntensityRange();
                _usSliceRenderer.p_transferFunction.getTF()->setIntensityDomain(cgt::vec2(ii.getLeft(), ii.getRight()));
                _usSliceRenderer.p_sliceNumber.setValue(125);
            }
        }
    }

}
