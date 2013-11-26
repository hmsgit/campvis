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

#include "AdvDVRVis.h"

#include "tgt/event/keyevent.h"
#include "core/datastructures/imagedata.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    AdvDVRVis::AdvDVRVis(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _camera("camera", "Camera")
        , _imageReader()
        , _pgGenerator()
        , _vmgGenerator()
        , _vmRenderer(&_canvasSize)
        , _eepGenerator(&_canvasSize)
        , _vmEepGenerator(&_canvasSize)
        , _dvrNormal(&_canvasSize)
        , _dvrVM(&_canvasSize)
        , _depthDarkening(&_canvasSize)
        , _combine(&_canvasSize)
        , _trackballEH(0)
    {
        addProperty(&_camera);

        _trackballEH = new TrackballNavigationEventListener(&_camera, &_canvasSize);
        _trackballEH->addLqModeProcessor(&_dvrNormal);
        _trackballEH->addLqModeProcessor(&_dvrVM);
        _trackballEH->addLqModeProcessor(&_depthDarkening);
        addEventListenerToBack(_trackballEH);

        addProcessor(&_imageReader);
        addProcessor(&_pgGenerator);
        addProcessor(&_vmgGenerator);
        addProcessor(&_vmRenderer);
        addProcessor(&_eepGenerator);
        addProcessor(&_vmEepGenerator);
        addProcessor(&_dvrNormal);
        addProcessor(&_dvrVM);
        addProcessor(&_depthDarkening);
        addProcessor(&_combine);
    }

    AdvDVRVis::~AdvDVRVis() {
        delete _trackballEH;
    }

    void AdvDVRVis::init() {
        AutoEvaluationPipeline::init();
        
        _imageReader.s_validated.connect(this, &AdvDVRVis::onProcessorValidated);

        _camera.addSharedProperty(&_vmgGenerator.p_camera);
        _camera.addSharedProperty(&_vmRenderer.p_camera);
        _camera.addSharedProperty(&_eepGenerator.p_camera);
        _camera.addSharedProperty(&_vmEepGenerator.p_camera);
        _camera.addSharedProperty(&_dvrNormal.p_camera);
        _camera.addSharedProperty(&_dvrVM.p_camera);

        //_imageReader.p_url.setValue("D:\\Medical Data\\Dentalscan\\dental.mhd");
        //_imageReader.p_url.setValue("C:/NavabJob/Dataset/nucleon.mhd");
        //_imageReader.p_url.setValue("C:/NavabJob/Dataset/walnut.mhd");
        //_imageReader.p_url.setValue("C:/NavabJob/Dataset/dental.mhd");
        _imageReader.p_url.setValue("C:/Campvis/Dataset/smallHeart.mhd");
        _imageReader.p_targetImageID.setValue("reader.output");
        _imageReader.p_targetImageID.addSharedProperty(&_eepGenerator.p_sourceImageID);
        _imageReader.p_targetImageID.addSharedProperty(&_vmEepGenerator.p_sourceImageID);
        _imageReader.p_targetImageID.addSharedProperty(&_dvrVM.p_sourceImageID);
        _imageReader.p_targetImageID.addSharedProperty(&_dvrNormal.p_sourceImageID);
        _imageReader.p_targetImageID.addSharedProperty(&_pgGenerator.p_sourceImageID);

        _dvrNormal.p_targetImageID.setValue("drr.output");
        _dvrVM.p_targetImageID.setValue("dvr.output");

         Geometry1DTransferFunction* dvrTF = new Geometry1DTransferFunction(128, tgt::vec2(0.f, .05f));
         dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.4f, .42f), tgt::col4(255, 0, 0, 255), tgt::col4(255, 0, 0, 255)));
         dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.45f, .5f), tgt::col4(0, 255, 0, 255), tgt::col4(0, 255, 0, 255)));
         _dvrNormal.p_transferFunction.replaceTF(dvrTF);

         Geometry1DTransferFunction* vmTF = new Geometry1DTransferFunction(128, tgt::vec2(0.f, .05f));
         vmTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.4f, .42f), tgt::col4(255, 0, 0, 255), tgt::col4(255, 0, 0, 255)));
         vmTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.45f, .5f), tgt::col4(0, 255, 0, 255), tgt::col4(0, 255, 0, 255)));
         _dvrVM.p_transferFunction.replaceTF(vmTF);

        _vmRenderer.p_renderTargetID.addSharedProperty(&_combine.p_mirrorRenderID);
        _vmEepGenerator.p_entryImageID.setValue("vm.eep.entry");
        _vmEepGenerator.p_exitImageID.setValue("vm.eep.exit");
        _vmEepGenerator.p_enableMirror.setValue(true);

        // not the most beautiful way... *g*
        // this will all get better with scripting support.
        static_cast<BoolProperty*>(_vmEepGenerator.getProperty("applyMask"))->setValue(true);
        _vmRenderer.p_renderTargetID.addSharedProperty(static_cast<DataNameProperty*>(_vmEepGenerator.getProperty("maskID")));

        _renderTargetID.setValue("combine");

        _pgGenerator.p_geometryID.addSharedProperty(&_vmEepGenerator.p_geometryID);
        _pgGenerator.p_geometryID.addSharedProperty(&_eepGenerator.p_geometryID);
        _vmgGenerator.p_mirrorID.addSharedProperty(&_vmEepGenerator.p_mirrorID);
        _vmgGenerator.p_mirrorID.addSharedProperty(&_vmRenderer.p_geometryID);
        _vmgGenerator.p_mirrorCenter.setValue(tgt::vec3(0.f, 0.f, -20.f));
        _vmgGenerator.p_poi.setValue(tgt::vec3(40.f, 40.f, 40.f));
        _vmgGenerator.p_size.setValue(60.f);

        _eepGenerator.p_entryImageID.addSharedProperty(&_dvrNormal.p_entryImageID);
        _vmEepGenerator.p_entryImageID.addSharedProperty(&_dvrVM.p_entryImageID);

        _eepGenerator.p_exitImageID.addSharedProperty(&_dvrNormal.p_exitImageID);
        _vmEepGenerator.p_exitImageID.addSharedProperty(&_dvrVM.p_exitImageID);

        _dvrVM.p_targetImageID.addSharedProperty(&_combine.p_mirrorImageID);
        _combine.p_targetImageID.setValue("combine");

        _dvrNormal.p_targetImageID.addSharedProperty(&_depthDarkening.p_inputImage);
        _depthDarkening.p_outputImage.addSharedProperty(&_combine.p_normalImageID);
    }

    void AdvDVRVis::deinit() {
        _canvasSize.s_changed.disconnect(this);
        AutoEvaluationPipeline::deinit();
    }

    void AdvDVRVis::onProcessorValidated(AbstractProcessor* processor) {
        if (processor == &_imageReader) {
            // update camera
            ScopedTypedData<ImageData> img(*_data, _imageReader.p_targetImageID.getValue());
            if (img != 0) {
                _trackballEH->reinitializeCamera(img);
            }
        }
    }


}