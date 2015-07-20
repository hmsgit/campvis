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

#include "dvrvis.h"

#include "cgt/event/keyevent.h"
#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"
#include "core/datastructures/imagedata.h"

namespace campvis {

    DVRVis::DVRVis(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _tcp(&_canvasSize)
        , _lsp()
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
    {
        _tcp.addLqModeProcessor(&_dvrNormal);
        _tcp.addLqModeProcessor(&_dvrVM);
        _tcp.addLqModeProcessor(&_depthDarkening);
        addEventListenerToBack(&_tcp);

        addProcessor(&_tcp);
        addProcessor(&_lsp);
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

    DVRVis::~DVRVis() {
    }

    void DVRVis::init() {
        AutoEvaluationPipeline::init();

        _imageReader.p_url.setValue(ShdrMgr.completePath("/modules/vis/sampledata/smallHeart.mhd"));
        _imageReader.p_targetImageID.setValue("reader.output");
        _imageReader.p_targetImageID.addSharedProperty(&_eepGenerator.p_sourceImageID);
        _imageReader.p_targetImageID.addSharedProperty(&_vmEepGenerator.p_sourceImageID);
        _imageReader.p_targetImageID.addSharedProperty(&_dvrVM.p_sourceImageID);
        _imageReader.p_targetImageID.addSharedProperty(&_dvrNormal.p_sourceImageID);
        _imageReader.p_targetImageID.addSharedProperty(&_pgGenerator.p_sourceImageID);
        _imageReader.p_targetImageID.addSharedProperty(&_tcp.p_image);

        _dvrNormal.p_targetImageID.setValue("drr.output");
        _dvrVM.p_targetImageID.setValue("dvr.output");

        Geometry1DTransferFunction* dvrTF = new Geometry1DTransferFunction(128, cgt::vec2(0.f, .05f));
        dvrTF->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.12f, .15f), cgt::col4(85, 0, 0, 128), cgt::col4(255, 0, 0, 128)));
        dvrTF->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.19f, .28f), cgt::col4(89, 89, 89, 155), cgt::col4(89, 89, 89, 155)));
        dvrTF->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.41f, .51f), cgt::col4(170, 170, 128, 64), cgt::col4(192, 192, 128, 64)));
        _dvrNormal.p_transferFunction.replaceTF(dvrTF);

        Geometry1DTransferFunction* vmTF = new Geometry1DTransferFunction(128, cgt::vec2(0.f, .05f));
        vmTF->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.41f, .51f), cgt::col4(170, 170, 128, 64), cgt::col4(192, 192, 128, 64)));
        _dvrVM.p_transferFunction.replaceTF(vmTF);

        _vmRenderer.p_renderTargetID.addSharedProperty(&_combine.p_mirrorRenderID);
        _vmEepGenerator.p_entryImageID.setValue("vm.eep.entry");
        _vmEepGenerator.p_exitImageID.setValue("vm.eep.exit");
        _vmEepGenerator.p_enableMirror.setValue(true);

        // not the most beautiful way... *g*
        // this will all get better with scripting support.
        static_cast<BoolProperty*>(_vmEepGenerator.getProperty("applyMask"))->setValue(true);
        _vmRenderer.p_renderTargetID.addSharedProperty(static_cast<DataNameProperty*>(_vmEepGenerator.getProperty("maskID")));
        _vmRenderer.p_renderMode.selectByOption(GL_TRIANGLE_FAN);

        _renderTargetID.setValue("combine");

        _pgGenerator.p_geometryID.addSharedProperty(&_vmEepGenerator.p_geometryID);
        _pgGenerator.p_geometryID.addSharedProperty(&_eepGenerator.p_geometryID);
        _vmgGenerator.p_mirrorID.addSharedProperty(&_vmEepGenerator.p_mirrorID);
        _vmgGenerator.p_mirrorID.addSharedProperty(&_vmRenderer.p_geometryID);
        _vmgGenerator.p_mirrorCenter.setValue(cgt::vec3(0.f, 0.f, -20.f));
        _vmgGenerator.p_poi.setValue(cgt::vec3(50.f, 80.f, 15.f));
        _vmgGenerator.p_size.setValue(128.f);

        _eepGenerator.p_entryImageID.addSharedProperty(&_dvrNormal.p_entryImageID);
        _vmEepGenerator.p_entryImageID.addSharedProperty(&_dvrVM.p_entryImageID);

        _eepGenerator.p_exitImageID.addSharedProperty(&_dvrNormal.p_exitImageID);
        _vmEepGenerator.p_exitImageID.addSharedProperty(&_dvrVM.p_exitImageID);

        _dvrVM.p_targetImageID.addSharedProperty(&_combine.p_mirrorImageID);
        _combine.p_targetImageID.setValue("combine");

        _dvrNormal.p_targetImageID.addSharedProperty(&_depthDarkening.p_inputImage);
        _depthDarkening.p_outputImage.addSharedProperty(&_combine.p_normalImageID);
    }

}