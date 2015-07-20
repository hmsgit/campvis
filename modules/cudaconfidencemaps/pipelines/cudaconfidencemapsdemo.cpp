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

#include "cudaconfidencemapsdemo.h"

#include <iomanip>
#include <sstream>
#include <QDateTime>
#include <QDir>
#include <QClipboard>
#include <QApplication>

#ifdef CAMPVIS_HAS_MODULE_DEVIL
#include <IL/il.h>
#include <IL/ilu.h>
#endif

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"
#include "cgt/event/keyevent.h"

namespace campvis {

    CudaConfidenceMapsDemo::CudaConfidenceMapsDemo(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _usIgtlReader()
        , _usCropFilter(&_canvasSize)
        , _usPreBlur(&_canvasSize)
        , _usBlurFilter(&_canvasSize)
        , _usResampler(&_canvasSize)
        , _usMapsSolver()
        , _usFusion(&_canvasSize)
        , _usFanRenderer(&_canvasSize)
        , p_useFixedIterationCount("UseFixedIterationCount", "Use Fixed Iteration Count", false)
        , p_millisecondBudget("MillisecondBudget", "(P)CG Milliseconds per frame", 24.0f, 10.0f, 60.0f)
        , p_iterationBudget("IterationBudget", "(P)CG Iteration Count", 100, 0, 1000)
        , p_connectDisconnectButton("ConnectToIGTLink", "Connect/Disconnect")
        , p_resamplingScale("ResampleScale", "Resample Scale", 0.5f, 0.01f, 1.0f)
        , p_beta("Beta", "Beta", 80.0f, 1.0f, 200.0f)
        , p_resetToDefault("ResetToDefault", "Reset Settings to Default")
        , p_collectStatistics("CollectStatistics", "Collect Statistics", false)
        , p_copyStatisticsToClipboard("CopyStatisticsToClipboard", "Copy Statistics To Clipboard as CSV")
        , p_showAdvancedOptions("ShowAdvancedOptions", "Advanced options...", false)
        , p_useAlphaBetaFilter("UseAlphaBetaFilter", "Alpha-Beta-Filter", true)
        , p_gaussianFilterSize("GaussianSigma", "Blur amount", 2.5f, 1.0f, 10.0f)
        , p_gradientScaling("GradientScaling", "Scaling factor for gradients", 2.0f, 0.001, 10)
        , p_alpha("Alpha", "Alpha", 2.0f, 0.0f, 10.0f)
        , p_gamma("Gamma", "Gamma", 0.05f, 0.0f, 0.4f, 0.001, 4)
        , p_fanHalfAngle("FanHalfAngle", "Fan Half Angle", 28.0f, 1.0f, 90.0f)
        , p_fanInnerRadius("FanInnerRadius", "Fan Inner Radius", 0.222f, 0.001f, 0.999f)
        , p_useSpacingEncodedFanGeometry("UseSpacingEncodedFanGeomtry", "Use spacing encoded fan geometry", true)
        , p_recordingDirectory("RecordingDirectory", "Recording output direcotry", "D:\\us_acquisitions\\")
        , p_enableRecording("EnableRecording", "Enable recording", false)
        , _recordedFrames(0)
        , _filePrefix(QDateTime::currentDateTime().toString("yyyyMMdd_hhmmss_").toStdString()) // Calculate file prefix, using date, hour, minute and second of when the pipeline was created
        , _statisticsLastUpdateTime()
    {
        addProcessor(&_usIgtlReader);
        addProcessor(&_usCropFilter);
        addProcessor(&_usPreBlur);
        addProcessor(&_usBlurFilter);
        addProcessor(&_usResampler);
        addProcessor(&_usMapsSolver);
        addProcessor(&_usFusion);
        addProcessor(&_usFanRenderer);

        addProperty(p_useFixedIterationCount);
        addProperty(p_millisecondBudget);
        addProperty(p_iterationBudget);
        addProperty(p_connectDisconnectButton);
        addProperty(p_resamplingScale);
        addProperty(p_beta);
        addProperty(p_resetToDefault);
        addProperty(p_collectStatistics);
        addProperty(p_copyStatisticsToClipboard);

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

        addProperty(p_useSpacingEncodedFanGeometry);

        setAdvancedPropertiesVisibility(false);

        _canvasSize.setVisible(false);
        _renderTargetID.setVisible(false);

        // Reserve memory for statistics, so that (hopefully) no reallocation happens at runtime
        _statistics.reserve(1000);
        _objectCreationTime = tbb::tick_count::now();
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
        _usCropFilter.p_outputImage.setValue("us.cropped");

        _usPreBlur.p_inputImage.setValue("us.cropped");
        _usPreBlur.p_outputImage.setValue("us");
        _usPreBlur.p_sigma.setValue(.6f);

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

        // Bind buttons to event handlers
        p_connectDisconnectButton.s_clicked.connect(this, &CudaConfidenceMapsDemo::toggleIGTLConnection);
        p_resetToDefault.s_clicked.connect(this, &CudaConfidenceMapsDemo::resetSettingsToDefault);
        p_copyStatisticsToClipboard.s_clicked.connect(this, &CudaConfidenceMapsDemo::copyStatisticsToClipboard);

        // Bind pipeline proeprties to processor properties
        p_useFixedIterationCount.addSharedProperty(&_usMapsSolver.p_useFixedIterationCount);
        p_millisecondBudget.addSharedProperty(&_usMapsSolver.p_millisecondBudget);
        p_iterationBudget.addSharedProperty(&_usMapsSolver.p_iterationBudget);

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
            auto startTime = tbb::tick_count::now();

            // Make sure that the whole pipeline gets invalidated and executed
            _usBlurFilter.invalidate(AbstractProcessor::INVALID_RESULT);
            _usCropFilter.invalidate(AbstractProcessor::INVALID_RESULT);
            _usResampler.invalidate(AbstractProcessor::INVALID_RESULT);
            _usMapsSolver.invalidate(AbstractProcessor::INVALID_RESULT);
            _usFusion.invalidate(AbstractProcessor::INVALID_RESULT);
            executeProcessorAndCheckOpenGLState(&_usIgtlReader);
            executeProcessorAndCheckOpenGLState(&_usCropFilter);
            executeProcessorAndCheckOpenGLState(&_usPreBlur);
            executeProcessorAndCheckOpenGLState(&_usBlurFilter);
            executeProcessorAndCheckOpenGLState(&_usResampler);
            executeProcessorAndCheckOpenGLState(&_usMapsSolver);

            // Read fan geomtry from encoded image...
            if (p_useSpacingEncodedFanGeometry.getValue()) {
                ImageRepresentationGL::ScopedRepresentation img(*_data, _usCropFilter.p_inputImage.getValue());
                auto image = reinterpret_cast<const ImageData*>(_data->getData(_usCropFilter.p_inputImage.getValue()).getData());
                if (image != nullptr) {
                    cgt::vec3 encodedData = image->getMappingInformation().getVoxelSize();
                    p_fanHalfAngle.setValue(encodedData.x / 2.0f);
                    p_fanInnerRadius.setValue(encodedData.y);
                }
            }

            executeProcessorAndCheckOpenGLState(&_usFusion);
            executeProcessorAndCheckOpenGLState(&_usFanRenderer);
                
            auto endTime = tbb::tick_count::now();

            if ((startTime - _statisticsLastUpdateTime).seconds() > 0.5f) {
                _statisticsLastUpdateTime = startTime;

                auto ms = (endTime - startTime).seconds() * 1000.0f;
                auto solverMs = _usMapsSolver.getActualSolverExecutionTime();
                std::stringstream string;
                string << "Mode: " << _usFusion.p_view.getOptionValue() << std::endl;
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

            // Collect statistics
            if (p_collectStatistics.getValue()) {
                StatisticsEntry entry;
                entry.time = (startTime - _objectCreationTime).seconds() * 1000.0f;

                ImageRepresentationGL::ScopedRepresentation originalImage(*_data, _usCropFilter.p_outputImage.getValue());
                ImageRepresentationGL::ScopedRepresentation downsampledImage(*_data, _usResampler.p_outputImage.getValue());

                if (originalImage && downsampledImage) {
                    entry.originalWidth = originalImage->getSize().x;
                    entry.originalHeight = originalImage->getSize().y;
                    entry.downsampledWidth = downsampledImage->getSize().x;
                    entry.downsampledHeight = downsampledImage->getSize().y;
                } else {
                    LWARNING("Could not read image size");
                    entry.originalWidth = entry.originalHeight = -1;
                    entry.downsampledWidth = entry.downsampledHeight = -1;
                }

                entry.gaussianKernelSize = _usBlurFilter.p_sigma.getValue();
                entry.scalingFactor = _usResampler.p_resampleScale.getValue();
                entry.alpha = _usMapsSolver.p_paramAlpha.getValue();
                entry.beta = _usMapsSolver.p_paramBeta.getValue();
                entry.gamma = _usMapsSolver.p_paramGamma.getValue();
                entry.gradientScaling = _usMapsSolver.p_gradientScaling.getValue();
                entry.iterations = _usMapsSolver.getActualConjugentGradientIterations();
                entry.solverExecutionTime = _usMapsSolver.getActualSolverExecutionTime();
                entry.totalExecutionTime = (endTime - startTime).seconds() * 1000.0f;
                entry.solverError = _usMapsSolver.getResidualNorm();

                _statistics.push_back(entry);
            }
        }
    }

    void CudaConfidenceMapsDemo::onEvent(cgt::Event* e) {
        // Allow for rapid switching between different visualizations
        //  F1: Ultrasound only
        //  F2: Sharpness
        //  F3: LAB
        //  F4: Color overlay
        //  F5: CM only
        if (typeid(*e) == typeid(cgt::KeyEvent)) {
            cgt::KeyEvent *keyEvent = reinterpret_cast<cgt::KeyEvent*>(e);
            if (keyEvent == nullptr) return;

            if (keyEvent->pressed()) {
                bool eventHandled = true;
                switch (keyEvent->keyCode()) {
                case cgt::KeyEvent::K_F1:
                    _usFusion.p_view.setValue(0); // US only
                    break;
                case cgt::KeyEvent::K_F2:
                    _usFusion.p_view.setValue(10); // Sharpness
                    {
                        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(256);
                        tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(0.0f, 1.0f), cgt::col4(0, 0, 0, 255), cgt::col4(0, 0, 0, 0)));
                        _usFusion.p_confidenceTF.replaceTF(tf);
                    }
                    break;
                case cgt::KeyEvent::K_F3:
                    _usFusion.p_view.setValue(8); // LAB
                    {
                        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(256);
                        tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(0.0f, 0.5f), cgt::col4(0, 0, 0, 192), cgt::col4(0, 0, 0, 0)));
                        _usFusion.p_confidenceTF.replaceTF(tf);
                        _usFusion.p_hue.setValue(0.23f);
                    }
                    break;
                case cgt::KeyEvent::K_F4:
                    _usFusion.p_view.setValue(12); // Color overlay
                    {
                        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(256);
                        tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(0.0f, 0.5f), cgt::col4(0, 0, 0, 255), cgt::col4(0, 0, 0, 0)));
                        _usFusion.p_confidenceTF.replaceTF(tf);
                        _usFusion.p_hue.setValue(0.15f);
                    }
                    break;
                case cgt::KeyEvent::K_F5:
                    _usFusion.p_view.setValue(2); // CM Only
                    break;
                default:
                    eventHandled = false;
                };
                if (eventHandled) {
                    e->accept();
                    // Force HUD statistics to be updated
                    _statisticsLastUpdateTime = tbb::tick_count();
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

        bool useFixedIterationCount = p_useFixedIterationCount.getValue();
        p_millisecondBudget.setVisible(!useFixedIterationCount);
        p_iterationBudget.setVisible(useFixedIterationCount);

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

    void CudaConfidenceMapsDemo::resetSettingsToDefault() {
        p_resamplingScale.setValue(.5f);
        p_gaussianFilterSize.setValue(2.5f);
        p_gradientScaling.setValue(2.f);
        p_alpha.setValue(2.f);
        p_beta.setValue(80.f);
        p_gamma.setValue(.05f);
    }

    void CudaConfidenceMapsDemo::copyStatisticsToClipboard() {
        // Copy statistics to the clipboard in CSV format
        std::cout << "Have " << _statistics.size() << " stats items" << std::endl;

        std::stringstream stream;
        stream << "time, originalWidth, originalHeight, downsampledWidth, downsampledHeight, gaussianKernelSize, scalingFactor, alpha, beta, gamma, gradientScaling, iterations, solverExecutionTime, totalExecutionTime, solverError" << std::endl;

        for (auto it = _statistics.begin(); it != _statistics.end(); ++it) {
            auto& item = *it;
            stream << item.time << ", ";
            stream << item.originalWidth << ", ";
            stream << item.originalHeight << ", ";
            stream << item.downsampledWidth << ", ";
            stream << item.downsampledHeight << ", ";
            stream << item.gaussianKernelSize << ", ";
            stream << item.scalingFactor << ", ";
            stream << item.alpha << ", ";
            stream << item.beta << ", ";
            stream << item.gamma << ", ";
            stream << item.gradientScaling << ", ";
            stream << item.iterations << ", ";
            stream << item.solverExecutionTime << ", ";
            stream << item.totalExecutionTime << ", ";
            stream << item.solverError << std::endl;
        }

        // Copy CSV stream to clipboard
        QClipboard *clipboard = QApplication::clipboard();
        QString text = QString::fromStdString(stream.str());
        clipboard->setText(text);

        _statistics.clear();
        _statistics.reserve(1000);
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