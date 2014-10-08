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

#include "mprdemo.h"

#include "tgt/event/keyevent.h"
#include "core/datastructures/imagedata.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    MprDemo::MprDemo(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _camera("camera", "Camera")
        , _lsp()
        , _imageReader()
        , _mprRenderer(&_canvasSize)
        , _compositor(&_canvasSize)
        , _trackballEH(0)
    {
        addProperty(_camera);

        _trackballEH = new TrackballNavigationEventListener(&_camera, &_canvasSize);
        addEventListenerToBack(_trackballEH);

        addProcessor(&_lsp);
        addProcessor(&_imageReader);
        addProcessor(&_mprRenderer);
        addProcessor(&_compositor);
    }

    MprDemo::~MprDemo() {
        delete _trackballEH;
    }

    void MprDemo::init() {
        AutoEvaluationPipeline::init();
        
        _imageReader.s_validated.connect(this, &MprDemo::onProcessorValidated);

        _camera.addSharedProperty(&_mprRenderer.p_camera);
        _mprRenderer.p_targetImageID.setValue("MPR");
        _mprRenderer.p_targetImageID.addSharedProperty(&_compositor.p_firstImageId);
        _mprRenderer.p_planeSize.setValue(250.f);

        _compositor.p_compositingMethod.selectByOption(RenderTargetCompositor::CompositingModeFirst);
        _compositor.p_targetImageId.setValue("final");
        _renderTargetID.setValue("final");

        _imageReader.p_url.setValue(ShdrMgr.completePath("/modules/vis/sampledata/smallHeart.mhd"));
        _imageReader.p_targetImageID.setValue("reader.output");
        _imageReader.p_targetImageID.addSharedProperty(&_mprRenderer.p_sourceImageID);

        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(128, tgt::vec2(0.f, .08f));
        tf->addGeometry(TFGeometry1D::createQuad(tgt::vec2(0.f, 1.f), tgt::col4(0, 0, 0, 255), tgt::col4(255, 255, 255, 255)));
        _mprRenderer.p_transferFunction.replaceTF(tf);
    }

    void MprDemo::deinit() {
        _canvasSize.s_changed.disconnect(this);
        AutoEvaluationPipeline::deinit();
    }

    void MprDemo::onProcessorValidated(AbstractProcessor* processor) {
        if (processor == &_imageReader) {
            // update camera
            ScopedTypedData<ImageData> img(*_data, _imageReader.p_targetImageID.getValue());
            if (img != 0) {
                _trackballEH->reinitializeCamera(img);
            }
        }
    }


}