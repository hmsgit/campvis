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

#include <tbb/tick_count.h>

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
        , p_iterations("Iterations", "Number of CG Iterations", 150, 1, 500)
        , p_autoIterationCount("AutoIterationCount", "Estimate iteration count based on time slot", true)
        , p_timeSlot("TimeSlot", "Milliseconds per frame", 32.0f, 10.0f, 250.0f)
        , p_connectToIGTLinkServer("ConnectToIGTLink", "Connect/Disconnect to IGTLink")
        , p_gaussianFilterSize("GaussianSigma", "Blur amount", 2.5f, 1.0f, 10.0f)
        , p_resamplingScale("ResampleScale", "Resample Scale", 0.25f, 0.01f, 1.0f)
        , p_alpha("Alpha", "Alpha", 2.0f, 0.0f, 10.0f)
        , p_beta("Beta", "Beta", 20.0f, 1.0f, 200.0f)
        , p_gamma("Gamma", "Gamma", 0.03f, 0.0f, 0.4f, 0.001, 4)
        , p_useAlphaBetaFilter("UseAlphaBetaFilter", "Alpha-Beta-Filter", true)
        , p_fanHalfAngle("FanHalfAngle", "Fan Half Angle", 37.0f, 1.0f, 90.0f)
        , p_fanInnerRadius("FanInnerRadius", "Fan Inner Radius", 0.222f, 0.001f, 0.999f)
    {
        addProcessor(&_usIgtlReader);
        addProcessor(&_usBlurFilter);
        addProcessor(&_usResampler);
        addProcessor(&_usMapsSolver);
        addProcessor(&_usFusion);
        addProcessor(&_usFanRenderer);

        addProperty(p_iterations);
        addProperty(p_autoIterationCount);
        addProperty(p_timeSlot);
        addProperty(p_connectToIGTLinkServer);
        addProperty(p_gaussianFilterSize);
        addProperty(p_resamplingScale);
        addProperty(p_alpha);
        addProperty(p_beta);
        addProperty(p_gamma);
        addProperty(p_useAlphaBetaFilter);
        addProperty(p_fanHalfAngle);
        addProperty(p_fanInnerRadius);
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

        // Bind pipeline proeprties to processor properties
        p_iterations.addSharedProperty(&_usMapsSolver.p_iterations);
        //p_autoIterationCount.addSharedProperty();
        //p_timeSlot.addSharedProperty();
        p_connectToIGTLinkServer.addSharedProperty(&_usIgtlReader.p_connect);
        p_gaussianFilterSize.addSharedProperty(&_usBlurFilter.p_sigma);
        p_resamplingScale.addSharedProperty(&_usResampler.p_resampleScale);
        p_alpha.addSharedProperty(&_usMapsSolver.p_paramAlpha);
        p_beta.addSharedProperty(&_usMapsSolver.p_paramBeta);
        p_gamma.addSharedProperty(&_usMapsSolver.p_paramGamma);
        p_useAlphaBetaFilter.addSharedProperty(&_usMapsSolver.p_useAlphaBetaFilter);
        p_fanHalfAngle.addSharedProperty(&_usFanRenderer.p_halfAngle);
        p_fanInnerRadius.addSharedProperty(&_usFanRenderer.p_innerRadius);
    }

    void CudaConfidenceMapsDemo::deinit() {
        AutoEvaluationPipeline::deinit();
    }

    void CudaConfidenceMapsDemo::executePipeline() {
        static float iterationsPerMsAverage = 1.0f;
        static unsigned char frame = 0;

        if (p_autoIterationCount.getValue() == false) {
            AutoEvaluationPipeline::executePipeline();
        }
        else {
            // Only launch the pipeline if the IgtlReader has recieved new data
            if (!_usIgtlReader.isValid()) {
                auto startTime = tbb::tick_count::now();

                // Make sure that the whole pipeline gets invalidated
                _usBlurFilter.invalidate(AbstractProcessor::INVALID_RESULT);
                _usBlurFilter.invalidate(AbstractProcessor::INVALID_RESULT);
                _usResampler.invalidate(AbstractProcessor::INVALID_RESULT);
                _usMapsSolver.invalidate(AbstractProcessor::INVALID_RESULT);
                _usFusion.invalidate(AbstractProcessor::INVALID_RESULT);

                executeProcessorAndCheckOpenGLState(&_usIgtlReader);
                executeProcessorAndCheckOpenGLState(&_usBlurFilter);
                executeProcessorAndCheckOpenGLState(&_usResampler);

                auto solverStartTime = tbb::tick_count::now();
                executeProcessorAndCheckOpenGLState(&_usMapsSolver);
                executeProcessorAndCheckOpenGLState(&_usFusion);
                executeProcessorAndCheckOpenGLState(&_usFanRenderer);

                if (frame % 16 == 0) {
                    tbb::tick_count endTime = tbb::tick_count::now();
                    auto ms = (endTime - startTime).seconds() * 1000.0f;
                    std::stringstream string;
                    string << "Execution time: " << static_cast<int>(ms) << "ms" << std::endl;
                    string << "CG Iterations: " << _usMapsSolver.getActualConjugentGradientIterations() << std::endl;
                    string << "Error: " << _usMapsSolver.getResidualNorm() << std::endl;
                    _usFanRenderer.p_text.setValue(string.str());
                }
                frame++;


                // Get the total end time (including fanRenderer) and calculate the number of
                // cg iterations that can be afforded
                auto endTime = tbb::tick_count::now();
                auto ms = (endTime - startTime).seconds() * 1000.0f;
                auto iterationsPerMs = _usMapsSolver.getActualConjugentGradientIterations() / ms;

                float mixRatio = 0.85f;
                iterationsPerMsAverage = iterationsPerMsAverage * mixRatio + iterationsPerMs * (1.0f - mixRatio);

                // Factor out the time needed for preprocessing the image from the time slot
                float timeSlot = (p_timeSlot.getValue() - (solverStartTime - startTime).seconds() / 1000.0f);
                int iterations = std::round(p_timeSlot.getValue() * iterationsPerMsAverage);
                p_iterations.setValue(iterations);
            }
        }
    }

    void CudaConfidenceMapsDemo::onRenderTargetSizeChanged(const AbstractProperty *prop) {
    }
}