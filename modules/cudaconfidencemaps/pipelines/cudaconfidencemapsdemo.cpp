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

#include "cudaconfidencemapsdemo.h"

#include "core/datastructures/imagedata.h"
#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    CudaConfidenceMapsDemo::CudaConfidenceMapsDemo(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _usIgtlReader()
        , _usBlurFilter(&_canvasSize)
        , _usResampler(&_canvasSize)
        , _usMapsSolver()
        , _usFusion(&_canvasSize)
        , _usFanRenderer(&_canvasSize)
    {
        addProcessor(&_usIgtlReader);
        addProcessor(&_usBlurFilter);
        addProcessor(&_usResampler);
        addProcessor(&_usMapsSolver);
        addProcessor(&_usFusion);
        addProcessor(&_usFanRenderer);
    }

    CudaConfidenceMapsDemo::~CudaConfidenceMapsDemo() {
    }

    void CudaConfidenceMapsDemo::init() {
        AutoEvaluationPipeline::init();

        // Set intial options
        _usIgtlReader.p_receiveImages.setValue(true);
        _usIgtlReader.p_receiveTransforms.setValue(false);
        _usIgtlReader.p_receivePositions.setValue(false);
        _usResampler.p_resampleScale.setValue(0.25f);
        // Set transfer function
        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(256);
        tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(0.0f, 0.5f), cgt::col4(0, 0, 0, 255), cgt::col4(0, 0, 0, 0)));
        _usFusion.p_confidenceTF.replaceTF(tf);

        // Create connectors
        _usIgtlReader.p_targetImagePrefix.setValue("us.igtl.");

        _usBlurFilter.p_inputImage.setValue("us.igtl.ImageClient");
        _usBlurFilter.p_outputImage.setValue("us.blurred");
        _usBlurFilter.p_outputImage.addSharedProperty(&_usResampler.p_inputImage);
        _usBlurFilter.p_outputImage.addSharedProperty(&_usFusion.p_blurredImageId);

        _usResampler.p_outputImage.setValue("us.resampled");
        _usResampler.p_outputImage.addSharedProperty(&_usMapsSolver.p_inputImage);

        _usMapsSolver.p_outputConfidenceMap.setValue("us.confidence");
        _usMapsSolver.p_outputConfidenceMap.addSharedProperty(&_usFusion.p_confidenceImageID);

        _usFusion.p_usImageId.setValue("us.igtl.ImageClient");
        _usFusion.p_targetImageID.setValue("us.fusion");
        _usFusion.p_view.setValue(12);
        _usFusion.p_renderToTexture.setValue(true);
        _usFusion.p_targetImageID.addSharedProperty(&_usFanRenderer.p_inputImage);



        _usFanRenderer.p_renderTargetID.setValue("us.fused_fan");
        _usFanRenderer.p_innerRadius.setValue(120.0f/540.0f);
        _usFanRenderer.p_halfAngle.setValue(37);

        _renderTargetID.setValue("us.fused_fan");
    }

    void CudaConfidenceMapsDemo::deinit() {
        AutoEvaluationPipeline::deinit();
    }

    void CudaConfidenceMapsDemo::onRenderTargetSizeChanged(const AbstractProperty *prop) {
    }
}