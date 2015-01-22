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

#include <iomanip>
#include <sstream>
#include <QDateTime>
#include <QDir>

#ifdef CAMPVIS_HAS_MODULE_DEVIL
#include <IL/il.h>
#include <IL/ilu.h>
#endif

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    CudaConfidenceMapsDemo::CudaConfidenceMapsDemo(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _usIgtlReader()
        , _usCropFilter(&_canvasSize)
        , _usBlurFilter(&_canvasSize)
        , _usResampler(&_canvasSize)
        , _usMapsSolver()
        , _usFusion(&_canvasSize)
        , _usFanRenderer(&_canvasSize)
        , p_millisecondBudget("MillisecondBudget", "Milliseconds per frame", 24.0f, 10.0f, 60.0f)
        , p_connectDisconnectButton("ConnectToIGTLink", "Connect/Disconnect")
        , p_resamplingScale("ResampleScale", "Resample Scale", 0.25f, 0.01f, 1.0f)
        , p_beta("Beta", "Beta", 20.0f, 1.0f, 200.0f)
        , p_showAdvancedOptions("ShowAdvancedOptions", "Advanced options...", false)
        , p_useAlphaBetaFilter("UseAlphaBetaFilter", "Alpha-Beta-Filter", true)
        , p_gaussianFilterSize("GaussianSigma", "Blur amount", 2.5f, 1.0f, 10.0f)
        , p_gradientScaling("GradientScaling", "Scaling factor for gradients", 2.0f, 0.001, 10)
        , p_alpha("Alpha", "Alpha", 2.0f, 0.0f, 10.0f)
        , p_gamma("Gamma", "Gamma", 0.03f, 0.0f, 0.4f, 0.001, 4)
        , p_fanHalfAngle("FanHalfAngle", "Fan Half Angle", 37.0f, 1.0f, 90.0f)
        , p_fanInnerRadius("FanInnerRadius", "Fan Inner Radius", 0.222f, 0.001f, 0.999f)
        , p_recordingDirectory("RecordingDirectory", "Recording output direcotry", "D:\\us_acquisitions\\")
        , p_enableRecording("EnableRecording", "Enable recording", false)
        , _recordedFrames(0)
        , _statisticsLastUpdateTime()
    {
        // Calculate file prefix, using date, hour, minute and second of when the pipeline was created
        _filePrefix = QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_").toStdString();

        addProcessor(&_usIgtlReader);
        addProcessor(&_usCropFilter);
        addProcessor(&_usBlurFilter);
        addProcessor(&_usResampler);
        addProcessor(&_usMapsSolver);
        addProcessor(&_usFusion);
        addProcessor(&_usFanRenderer);

        addProperty(p_millisecondBudget);
        addProperty(p_connectDisconnectButton);
        addProperty(p_resamplingScale);
        addProperty(p_beta);

        addProperty(p_showAdvancedOptions);

        addProperty(p_useAlphaBetaFilter);
        addProperty(p_gaussianFilterSize);
        addProperty(p_gradientScaling);
        addProperty(p_alpha);
        addProperty(p_gamma);
        addProperty(p_fanHalfAngle);
        addProperty(p_fanInnerRadius);
        addProperty(p_recordingDirectory);
        addProperty(p_enableRecording);

        setAdvancedPropertiesVisibility(false);
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

        _usCropFilter.p_inputImage.setValue("us.igtl.CAMPUS");
        _usCropFilter.p_outputImage.setValue("us");

        _usBlurFilter.p_inputImage.setValue("us");
        _usBlurFilter.p_outputImage.setValue("us.blurred");
        _usBlurFilter.p_outputImage.addSharedProperty(&_usResampler.p_inputImage);
        _usBlurFilter.p_outputImage.addSharedProperty(&_usFusion.p_blurredImageId);

        _usResampler.p_outputImage.setValue("us.resampled");
        _usResampler.p_outputImage.addSharedProperty(&_usMapsSolver.p_inputImage);

        _usMapsSolver.p_outputConfidenceMap.setValue("us.confidence");
        _usMapsSolver.p_outputConfidenceMap.addSharedProperty(&_usFusion.p_confidenceImageID);

        _usFusion.p_usImageId.setValue("us");
        _usFusion.p_targetImageID.setValue("us.fusion");
        _usFusion.p_view.setValue(12);
        _usFusion.p_renderToTexture.setValue(true);
        _usFusion.p_targetImageID.addSharedProperty(&_usFanRenderer.p_inputImage);
        _usFusion.p_transferFunction.setAutoFitWindowToData(false);
        _usFusion.p_confidenceTF.setAutoFitWindowToData(false);

        _usFanRenderer.p_renderTargetID.setValue("us.fused_fan");
        _usFanRenderer.p_innerRadius.setValue(120.0f/540.0f);
        _usFanRenderer.p_halfAngle.setValue(37);

        _renderTargetID.setValue("us.fused_fan");

        // Bind pipeline proeprties to processor properties
        p_connectDisconnectButton.s_clicked.connect(this, &CudaConfidenceMapsDemo::toggleIGTLConnection);

        p_gaussianFilterSize.addSharedProperty(&_usBlurFilter.p_sigma);
        p_resamplingScale.addSharedProperty(&_usResampler.p_resampleScale);
        p_gradientScaling.addSharedProperty(&_usMapsSolver.p_gradientScaling);
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
        // Only launch the pipeline if the IgtlReader has recieved new data
        // FIXME: It would be better to check if a new image actually arrived, instead
        // of just checking the invaildation state of the IGTLReader
        if (!_usIgtlReader.isValid()) {
            float millisecondBudget = p_millisecondBudget.getValue();
            auto startTime = tbb::tick_count::now();

            // Make sure that the whole pipeline gets invalidated
            _usBlurFilter.invalidate(AbstractProcessor::INVALID_RESULT);
            _usCropFilter.invalidate(AbstractProcessor::INVALID_RESULT);
            _usResampler.invalidate(AbstractProcessor::INVALID_RESULT);
            _usMapsSolver.invalidate(AbstractProcessor::INVALID_RESULT);
            _usFusion.invalidate(AbstractProcessor::INVALID_RESULT);

            executeProcessorAndCheckOpenGLState(&_usIgtlReader);
            executeProcessorAndCheckOpenGLState(&_usCropFilter);
            executeProcessorAndCheckOpenGLState(&_usBlurFilter) ;
            executeProcessorAndCheckOpenGLState(&_usResampler);

            auto solverStartTime = tbb::tick_count::now();
            _usMapsSolver.p_millisecondBudget.setValue(millisecondBudget);
            executeProcessorAndCheckOpenGLState(&_usMapsSolver);
            auto solverEndTime = tbb::tick_count::now();

            executeProcessorAndCheckOpenGLState(&_usFusion);
            executeProcessorAndCheckOpenGLState(&_usFanRenderer);
                
            auto endTime = tbb::tick_count::now();

            if ((startTime - _statisticsLastUpdateTime).seconds() > 0.5f) {
                _statisticsLastUpdateTime = startTime;

                auto ms = (endTime - startTime).seconds() * 1000.0f;
                auto solverMs = (solverEndTime - solverStartTime).seconds() * 1000.0f;
                std::stringstream string;
                string << "Execution time: " << static_cast<int>(ms) << "ms" << std::endl;
                string << "Solver time: " << static_cast<int>(solverMs) << "ms" << std::endl;
                string << "CG Iterations: " << _usMapsSolver.getActualConjugentGradientIterations() << std::endl;
                string << "Error: " << _usMapsSolver.getResidualNorm() << std::endl;
                _usFanRenderer.p_text.setValue(string.str());
            }

            if (p_enableRecording.getValue()) {
                _recordedFrames++;
                QDir dir(QString::fromStdString(p_recordingDirectory.getValue()));
                std::stringstream filename;
                filename << _filePrefix << std::setw(4) << std::setfill('0') << _recordedFrames << ".png";
                auto url = dir.absoluteFilePath(QString::fromStdString(filename.str())).toStdString();

                ScopedTypedData<ImageData> rd(*_data, "us");
                const ImageRepresentationGL *rep = rd->getRepresentation<ImageRepresentationGL>();
                if (rep != 0) {
#ifdef CAMPVIS_HAS_MODULE_DEVIL
                    if (!cgt::FileSystem::dirExists(dir.absolutePath().toStdString()))
                        cgt::FileSystem::createDirectory(dir.absolutePath().toStdString());

                    // get color buffer content
                    GLubyte* colorBuffer = rep->getTexture()->downloadTextureToBuffer(GL_RED, GL_UNSIGNED_BYTE);
                    cgt::ivec2 size = rep->getSize().xy();

                    // create Devil image from image data and write it to file
                    ILuint img;
                    ilGenImages(1, &img);
                    ilBindImage(img);

                    // put pixels into IL-Image
                    ilTexImage(size.x, size.y, 1, 1, GL_LUMINANCE, IL_UNSIGNED_BYTE, colorBuffer);
                    ilEnable(IL_FILE_OVERWRITE);
                    ilResetWrite();
                    ILboolean success = ilSaveImage(url.c_str());
                    ilDeleteImages(1, &img);

                    delete[] colorBuffer;

                    if (!success) {
                        LERROR("Could not save image to file: " << ilGetError());
                    }
#endif
                }
            }
        }
    }

    void CudaConfidenceMapsDemo::onPropertyChanged(const AbstractProperty* prop) {
        if (prop == &p_showAdvancedOptions) {
            setAdvancedPropertiesVisibility(p_showAdvancedOptions.getValue());
            return;
        }
        else if (prop == &_renderTargetID && _renderTargetID.getValue() != "us.fused_fan") {
            // Prevent the program from changing the render target
            _renderTargetID.setValue("us.fused_fan");
        }
        
        AutoEvaluationPipeline::onPropertyChanged(prop);
    }

    void CudaConfidenceMapsDemo::toggleIGTLConnection() {
        // Simulate a click on the currently visible button of the IGTL reader,
        // causing it to either connect or disconnect from a server.
        if (_usIgtlReader.p_connect.isVisible())
            _usIgtlReader.p_connect.click();
        else
            _usIgtlReader.p_disconnect.click();
    }

    void CudaConfidenceMapsDemo::setAdvancedPropertiesVisibility(bool visible) {
        if (p_showAdvancedOptions.getValue() != visible) {
            p_showAdvancedOptions.setValue(visible);
        }

        p_useAlphaBetaFilter.setVisible(visible);
        p_gaussianFilterSize.setVisible(visible);
        p_gradientScaling.setVisible(visible);
        p_alpha.setVisible(visible);
        p_gamma.setVisible(visible);
        p_fanHalfAngle.setVisible(visible);
        p_fanInnerRadius.setVisible(visible);
        p_recordingDirectory.setVisible(visible);
        p_enableRecording.setVisible(visible);
    }

}