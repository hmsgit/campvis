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

#include "tensordemo.h"

#include "cgt/event/keyevent.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    TensorDemo::TensorDemo(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _tcp(&_canvasSize)
        , _lsp()
        , _imageReader()
        , _ta()
        , _glyphRenderer(&_canvasSize)
        , _sliceRenderer(&_canvasSize)
        , _rtc(&_canvasSize)
        , p_sliceNumber("SliceNuber", "Slice Number", 0, 0, 256)

    {
        addProperty(p_sliceNumber);

        addEventListenerToBack(&_tcp);

        addProcessor(&_tcp);
        addProcessor(&_lsp);
        addProcessor(&_imageReader);
        addProcessor(&_ta);
        addProcessor(&_glyphRenderer);
        addProcessor(&_sliceRenderer);
        addProcessor(&_rtc);
    }

    TensorDemo::~TensorDemo() {
    }

    void TensorDemo::init() {
        AutoEvaluationPipeline::init();

        p_sliceNumber.addSharedProperty(&_glyphRenderer.p_sliceNumber);
        p_sliceNumber.addSharedProperty(&_sliceRenderer.p_sliceNumber);

        _imageReader.p_url.setValue(CAMPVIS_SOURCE_DIR "/modules/tensor/sampledata/planar_tensor.mhd");
        _imageReader.p_targetImageID.setValue("reader.output");
        _imageReader.p_targetImageID.addSharedProperty(&_ta.p_inputImage);
        _imageReader.p_targetImageID.addSharedProperty(&_tcp.p_image);

        _ta.p_outputProperties[0]->_imageId.addSharedProperty(&_sliceRenderer.p_sourceImageID);
        _ta.p_outputProperties[0]->_imageType.selectById("Trace");
        _ta.p_evalsImage.addSharedProperty(&_glyphRenderer.p_inputEigenvalues);
        _ta.p_evecsImage.addSharedProperty(&_glyphRenderer.p_inputEigenvectors);

        _glyphRenderer.p_renderOutput.setValue("glyphs");
        _glyphRenderer.p_renderOutput.addSharedProperty(&_rtc.p_firstImageId);

        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(128, cgt::vec2(0.f, 1.f));
        tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(0.f, 1.f), cgt::col4(0, 0, 0, 255), cgt::col4(255, 255, 255, 255)));
        _sliceRenderer.p_transferFunction.replaceTF(tf);
        _sliceRenderer.p_targetImageID.setValue("slice");
        _sliceRenderer.p_targetImageID.addSharedProperty(&_rtc.p_secondImageId);

        _rtc.p_compositingMethod.selectById("depth");
        _rtc.p_targetImageId.setValue("composed");

        _renderTargetID.setValue("composed");
        
    }

}
