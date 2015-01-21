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
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imageseries.h"

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
        , _mvr(&_canvasSize)
    {
        _tcp.addLqModeProcessor(&_mvr);
        addEventListenerToBack(&_tcp);

        addProcessor(&_lsp);
        addProcessor(&_tcp);
        addProcessor(&_ctReader);
        addProcessor(&_t1Reader);
        addProcessor(&_petReader);
        addProcessor(&_mvr);
    }

    NeuroDemo::~NeuroDemo() {
    }

    void NeuroDemo::init() {
        AutoEvaluationPipeline::init();
        
        _tcp.p_image.setValue("ImageGroup");
        _renderTargetID.setValue("result");

        _ctReader.p_url.setValue(ShdrMgr.completePath("D:/Medical Data/K_export/K_Data/K_CT_CoregT1.am"));
        _ctReader.p_targetImageID.setValue("ct.image");
        _ctReader.s_validated.connect(this, &NeuroDemo::onReaderValidated);

        _t1Reader.p_url.setValue(ShdrMgr.completePath("D:/Medical Data/K_export/K_Data/K_T1_bet04.GB306.am"));
        _t1Reader.p_targetImageID.setValue("t1_tf.image");
        _t1Reader.s_validated.connect(this, &NeuroDemo::onReaderValidated);

        _petReader.p_url.setValue(ShdrMgr.completePath("D:/Medical Data/K_export/K_Data/K_PET-CoregNMI_fl.am"));
        _petReader.p_targetImageID.setValue("pet.image");
        _petReader.s_validated.connect(this, &NeuroDemo::onReaderValidated);

        Geometry1DTransferFunction* ct_tf = new Geometry1DTransferFunction(128, cgt::vec2(0.f, .05f));
        ct_tf->addGeometry(TFGeometry1D::crateRamp(cgt::vec2(.5f, .9f), cgt::col4(255, 255, 255, 255)));
        _mvr.p_transferFunction1.replaceTF(ct_tf);

        Geometry1DTransferFunction* t1_tf = new Geometry1DTransferFunction(128, cgt::vec2(0.f, .05f));
        t1_tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.12f, .15f), cgt::col4(85, 0, 0, 128), cgt::col4(255, 0, 0, 128)));
        t1_tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.19f, .28f), cgt::col4(89, 89, 89, 155), cgt::col4(89, 89, 89, 155)));
        t1_tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.41f, .51f), cgt::col4(170, 170, 128, 64), cgt::col4(192, 192, 128, 64)));
        _mvr.p_transferFunction2.replaceTF(t1_tf);

        Geometry1DTransferFunction* pet_tf = new Geometry1DTransferFunction(128, cgt::vec2(0.f, .05f));
        pet_tf->addGeometry(TFGeometry1D::crateRamp(cgt::vec2(.5f, .9f), cgt::col4(255, 255, 255, 255)));
        _mvr.p_transferFunction3.replaceTF(pet_tf);

        _mvr.p_sourceImagesId.setValue("ImageGroup");
        _mvr.p_outputImageId.setValue("result");
        _mvr.p_samplingRate.setValue(1.f);
    }

    void NeuroDemo::onReaderValidated(AbstractProcessor* p) {
        ScopedTypedData<ImageData> ctImage(getDataContainer(), _ctReader.p_targetImageID.getValue());
        ScopedTypedData<ImageData> t1Image(getDataContainer(), _t1Reader.p_targetImageID.getValue());
        ScopedTypedData<ImageData> petImage(getDataContainer(), _petReader.p_targetImageID.getValue());

        ImageSeries* is = new ImageSeries();
        if (ctImage)
            is->addImage(ctImage.getDataHandle());
        if (t1Image)
            is->addImage(t1Image.getDataHandle());
        if (petImage)
            is->addImage(petImage.getDataHandle());

        getDataContainer().addData("ImageGroup", is);
    }

}