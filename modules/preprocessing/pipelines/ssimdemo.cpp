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

#include "ssimdemo.h"

#include "cgt/shadermanager.h"


namespace campvis {

    SsimDemo::SsimDemo(DataContainer& dc)
        : AutoEvaluationPipeline(dc, getId())
        , _imageReader()
        , _gaussian(&_canvasSize)
        , _ssim(&_canvasSize)
    {
        addProcessor(&_imageReader);
        addProcessor(&_gaussian);
        addProcessor(&_ssim);
    }

    SsimDemo::~SsimDemo() {
    }

    void SsimDemo::init() {
        AutoEvaluationPipeline::init();

        _ssim.p_outputImage.setValue("result");
        _renderTargetID.setValue("result");

        _imageReader.p_url.setValue(ShdrMgr.completePath("/modules/"));
        _imageReader.p_targetImageID.setValue("reader.output");
        _imageReader.p_targetImageID.addSharedProperty(&_gaussian.p_inputImage);

        _gaussian.p_outputImage.setValue("blurred");
    }


}