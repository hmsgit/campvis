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

#include "tensordemo.h"

#include "tgt/event/keyevent.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    TensorDemo::TensorDemo(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _imageReader()
        , _ta()
        , _sliceExtractor(&_canvasSize)
        , _wheelHandler(&_sliceExtractor.p_zSliceNumber)
    {
        addProcessor(&_imageReader);
        addProcessor(&_ta);
        addProcessor(&_sliceExtractor);
        addEventListenerToBack(&_wheelHandler);
    }

    TensorDemo::~TensorDemo() {
    }

    void TensorDemo::init() {
        AutoEvaluationPipeline::init();

        _imageReader.p_url.setValue(CAMPVIS_SOURCE_DIR "/modules/tensor/sampledata/planar_tensor.mhd");
        _imageReader.p_targetImageID.setValue("reader.output");
        _imageReader.p_targetImageID.addSharedProperty(&_ta.p_inputImage);

        _ta.p_outputProperties[0]->_imageId.addSharedProperty(&_sliceExtractor.p_sourceImageID);
        _ta.p_outputProperties[0]->_imageType.selectById("MainEigenvector");

        _sliceExtractor.p_xSliceNumber.setValue(0);

        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(128, tgt::vec2(0.f, 1.f));
        tf->addGeometry(TFGeometry1D::createQuad(tgt::vec2(0.f, 1.f), tgt::col4(0, 0, 0, 0), tgt::col4(255, 255, 255, 255)));
        _sliceExtractor.p_transferFunction.replaceTF(tf);

        _renderTargetID.setValue("renderTarget");
        _renderTargetID.addSharedProperty(&(_sliceExtractor.p_targetImageID));
    }

}