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

#include "volumerendererdemo.h"

#include "cgt/event/keyevent.h"
#include "core/datastructures/imagedata.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    VolumeRendererDemo::VolumeRendererDemo(DataContainer& dc)
        : AutoEvaluationPipeline(dc, getId())
        , _tcp(&_canvasSize)
        , _lsp()
        , _imageReader()
        , _vr(&_canvasSize)
    {
        _tcp.addLqModeProcessor(&_vr);
        addEventListenerToBack(&_tcp);

        addProcessor(&_tcp);
        addProcessor(&_lsp);
        addProcessor(&_imageReader);
        addProcessor(&_vr);
    }

    VolumeRendererDemo::~VolumeRendererDemo() {
    }

    void VolumeRendererDemo::init() {
        AutoEvaluationPipeline::init();
        
        _vr.p_outputImage.setValue("combine");
        _renderTargetID.setValue("combine");

        _imageReader.p_url.setValue(ShdrMgr.completePath("/modules/vis/sampledata/smallHeart.mhd"));
        _imageReader.p_targetImageID.setValue("reader.output");
        _imageReader.p_targetImageID.addSharedProperty(&_tcp.p_image);
        _imageReader.p_targetImageID.addSharedProperty(&_vr.p_inputVolume);

        Geometry1DTransferFunction* dvrTF = new Geometry1DTransferFunction(128, cgt::vec2(0.f, .05f));
        dvrTF->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.12f, .15f), cgt::col4(85, 0, 0, 128), cgt::col4(255, 0, 0, 128)));
        dvrTF->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.19f, .28f), cgt::col4(89, 89, 89, 155), cgt::col4(89, 89, 89, 155)));
        dvrTF->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.41f, .51f), cgt::col4(170, 170, 128, 64), cgt::col4(192, 192, 128, 64)));
        static_cast<TransferFunctionProperty*>(_vr.getNestedProperty("RaycasterProps::TransferFunction"))->replaceTF(dvrTF);
        static_cast<FloatProperty*>(_vr.getNestedProperty("RaycasterProps::SamplingRate"))->setValue(4.f);
    }

}