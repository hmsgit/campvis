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

#include "cgt/event/keyevent.h"
#include "core/datastructures/imagedata.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    MprDemo::MprDemo(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _tcp(&_canvasSize)
        , _lsp()
        , _imageReader()
        , _mprRenderer(&_canvasSize)
        , _compositor(&_canvasSize)
    {
        addProcessor(&_tcp);
        addProcessor(&_lsp);
        addProcessor(&_imageReader);
        addProcessor(&_mprRenderer);
        addProcessor(&_compositor);

        addEventListenerToBack(&_tcp);
    }

    MprDemo::~MprDemo() {
    }

    void MprDemo::init() {
        AutoEvaluationPipeline::init();
        
        _mprRenderer.p_targetImageID.setValue("MPR");
        _mprRenderer.p_targetImageID.addSharedProperty(&_compositor.p_firstImageId);
        _mprRenderer.p_planeSize.setValue(250.f);

        _compositor.p_compositingMethod.selectByOption(RenderTargetCompositor::CompositingModeFirst);
        _compositor.p_targetImageId.setValue("final");
        _renderTargetID.setValue("final");

        _imageReader.p_url.setValue(ShdrMgr.completePath("/modules/vis/sampledata/smallHeart.mhd"));
        _imageReader.p_targetImageID.setValue("reader.output");
        _imageReader.p_targetImageID.addSharedProperty(&_tcp.p_image);
        _imageReader.p_targetImageID.addSharedProperty(&_mprRenderer.p_sourceImageID);

        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(128, cgt::vec2(0.f, .08f));
        tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(0.f, 1.f), cgt::col4(0, 0, 0, 255), cgt::col4(255, 255, 255, 255)));
        _mprRenderer.p_transferFunction.replaceTF(tf);
    }

}