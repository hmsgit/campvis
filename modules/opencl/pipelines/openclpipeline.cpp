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

#include "openclpipeline.h"

#include "tgt/event/keyevent.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    OpenCLPipeline::OpenCLPipeline(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _camera("camera", "Camera")
        , _imageReader()
        , _pgGenerator()
        , _eepGenerator(&_canvasSize)
        , _clRaycaster(&_canvasSize)
        , _trackballEH(0)
    {
        addProperty(&_camera);

        _trackballEH = new TrackballNavigationEventListener(&_camera, &_canvasSize);
        _trackballEH->addLqModeProcessor(&_clRaycaster);
        addEventListenerToBack(_trackballEH);

        addProcessor(&_imageReader);
        addProcessor(&_pgGenerator);
        addProcessor(&_eepGenerator);
        addProcessor(&_clRaycaster);
    }

    OpenCLPipeline::~OpenCLPipeline() {
        delete _trackballEH;
    }

    void OpenCLPipeline::init() {
        AutoEvaluationPipeline::init();

        _camera.addSharedProperty(&_eepGenerator.p_camera);
        _camera.addSharedProperty(&_clRaycaster._camera);

        //_imageReader.p_url.setValue("D:\\Medical Data\\Dentalscan\\dental.mhd");
        _imageReader.p_url.setValue("D:\\Medical Data\\smallHeart.mhd");
        _imageReader.p_targetImageID.setValue("reader.output");
        _imageReader.p_targetImageID.addSharedProperty(&_pgGenerator.p_sourceImageID);
        _imageReader.p_targetImageID.addSharedProperty(&_eepGenerator.p_sourceImageID);
        _imageReader.s_validated.connect(this, &OpenCLPipeline::onProcessorValidated);

        _clRaycaster._targetImageID.setValue("cl.output");
        _clRaycaster._sourceImageID.setValue("clr.input");

         Geometry1DTransferFunction* dvrTF = new Geometry1DTransferFunction(128, tgt::vec2(0.f, .05f));
         dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.4f, .42f), tgt::col4(255, 0, 0, 255), tgt::col4(255, 0, 0, 255)));
         dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.45f, .5f), tgt::col4(0, 255, 0, 255), tgt::col4(0, 255, 0, 255)));
         _clRaycaster._transferFunction.replaceTF(dvrTF);



        _renderTargetID.setValue("cl.output");

        _pgGenerator.p_geometryID.addSharedProperty(&_eepGenerator.p_geometryID);

        _eepGenerator.p_entryImageID.addSharedProperty(&_clRaycaster._entryImageID);
        _eepGenerator.p_exitImageID.addSharedProperty(&_clRaycaster._exitImageID);
    }

    void OpenCLPipeline::deinit() {
        _canvasSize.s_changed.disconnect(this);
        AutoEvaluationPipeline::deinit();
    }

    void OpenCLPipeline::onProcessorValidated(AbstractProcessor* processor) {
        if (processor == &_imageReader) {
            // update camera
            ImageRepresentationLocal::ScopedRepresentation img(*_data, "reader.output");
            if (img != 0) {
                size_t numElements = img->getNumElements();
                float* asFloats = new float[numElements];
                for (size_t i = 0; i < numElements; ++i)
                    asFloats[i] = img->getElementNormalized(i, 0);
                ImageData* id = new ImageData(img->getDimensionality(), img->getSize(), img->getParent()->getNumChannels());
                GenericImageRepresentationLocal<float, 1>* imageWithFloats = GenericImageRepresentationLocal<float, 1>::create(id, asFloats);
                DataHandle dh = _data->addData("clr.input", id);
                _trackballEH->reinitializeCamera(img->getParent());
            }
        }
    }



}