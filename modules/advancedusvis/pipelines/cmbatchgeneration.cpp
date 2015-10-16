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

#include "cmbatchgeneration.h"

#include "cgt/filesystem.h"
#include "cgt/glcontextmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/event/keyevent.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/facegeometry.h"

#ifdef CAMPVIS_HAS_MODULE_DEVIL
#include <IL/il.h>
#include <IL/ilu.h>
#endif

#include <sstream>
#include <iomanip>

namespace campvis {

    CmBatchGeneration::CmBatchGeneration(DataContainer& dc)
        : AutoEvaluationPipeline(dc, getId())
        , _usReader()
        , _confidenceGenerator()
        , _usBlurFilter(&_canvasSize)
        , _usFusion(&_canvasSize)
        , p_autoExecution("AutoExecution", "Automatic Execution", false)
        , p_showFan("ShowFan", "Show Fan", true)
        , p_sourcePath("SourcePath", "Source Files Path", "", StringProperty::DIRECTORY)
        , p_targetPathResampled("TargetPathResampled", "Target Path Resampled Files", "", StringProperty::DIRECTORY)
        , p_targetPathCmCpu("TargetPathCm", "Target Path Confidence Map Files", "", StringProperty::DIRECTORY)
        , p_targetPathColorOverlay("TargetPathColorOverlay", "Target Path Color Overlay Files", "", StringProperty::DIRECTORY)
        , p_targetPathColor("TargetPathColor", "Target Path Color Files", "", StringProperty::DIRECTORY)
        , p_targetPathFuzzy("TargetPathFuzzy", "Target Path Fuzzy Files", "", StringProperty::DIRECTORY)
        , p_range("Range", "Files Range", cgt::ivec2(0, 1), cgt::ivec2(0, 0), cgt::ivec2(10000, 10000))
        , p_execute("Execute", "Execute Batch Pipeline")
        , _shader(nullptr)
    {
        addProcessor(&_usReader);
        addProcessor(&_scanlineConverter);
        addProcessor(&_confidenceGenerator);
        addProcessor(&_usFusion);
        addProcessor(&_usBlurFilter);

        addProperty(p_autoExecution);
        addProperty(p_sourcePath);
        addProperty(p_targetPathResampled);
        addProperty(p_targetPathCmCpu);
        addProperty(p_targetPathColorOverlay);
        addProperty(p_targetPathColor);
        addProperty(p_targetPathFuzzy);
        addProperty(p_range);
        addProperty(p_execute);
    }

    CmBatchGeneration::~CmBatchGeneration() {
    }

    void CmBatchGeneration::init() {
        AutoEvaluationPipeline::init();
        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "core/glsl/passthrough.frag", "");
        _imageWriter.init();

        p_sourcePath.setValue("D:\\cm_stuff\\original");
        p_targetPathResampled.setValue("D:\\cm_stuff\\resampled");
        p_targetPathCmCpu.setValue("D:\\cm_stuff\\cm");
        p_targetPathColorOverlay.setValue("D:\\cm_stuff\\colorOverlay");
        p_targetPathColor.setValue("D:\\cm_stuff\\color");
        p_targetPathFuzzy.setValue("D:\\cm_stuff\\fuzzy");
        p_range.setValue(cgt::ivec2(0, 1));
        p_execute.s_clicked.connect(this, &CmBatchGeneration::startBatchProcess);

        _usReader.p_url.setValue("D:\\cm_stuff\\original\\export0000.bmp");
        _usReader.p_targetImageID.setValue("us.image");
        _usReader.p_importType.selectById("localIntensity");
        _usReader.p_targetImageID.addSharedProperty(&_scanlineConverter.p_sourceImageID);

        _scanlineConverter.p_targetImageID.setValue("us.resampled");
        _scanlineConverter.p_targetImageID.addSharedProperty(&_confidenceGenerator.p_sourceImageID);
        _scanlineConverter.p_targetImageID.addSharedProperty(&_usFusion.p_usImageId);
        _scanlineConverter.p_targetImageID.addSharedProperty(&_usBlurFilter.p_inputImage);

        _confidenceGenerator.p_targetImageID.setValue("confidence.image.generated");
        _confidenceGenerator.p_targetImageID.addSharedProperty(&_usFusion.p_confidenceImageID);
        //_confidenceGenerator.p_curvilinear.setValue(true);
        //_confidenceGenerator.p_origin.setValue(cgt::vec2(340.f, 540.f));
        //_confidenceGenerator.p_angles.setValue(cgt::vec2(4.064f, 5.363f));
        ////_confidenceGenerator.p_angles.setValue(cgt::vec2(232.f / 180.f * cgt::PIf, 307.f / 180.f * cgt::PIf));
        ////_confidenceGenerator.p_origin.setValue(cgt::vec2(320.f, 35.f));
        ////_confidenceGenerator.p_angles.setValue(cgt::vec2(45.f / 180.f * cgt::PIf, 135.f / 180.f * cgt::PIf));
        //_confidenceGenerator.p_lengths.setValue(cgt::vec2(116.f, 543.f));
        _confidenceGenerator.p_alpha.setValue(2.f);
        _confidenceGenerator.p_beta.setValue(80.f);
        _confidenceGenerator.p_gamma.setValue(.05f);


        _usFusion.p_targetImageID.setValue("us.fused");
        _usFusion.p_view.selectById("mappingSharpness");
        _usFusion.p_sliceNumber.setValue(0);

        _usBlurFilter.p_outputImage.setValue("us.blurred");
        _usBlurFilter.p_outputImage.addSharedProperty(&_usFusion.p_blurredImageId);
        _usBlurFilter.p_sigma.setValue(2.f);


        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(128, cgt::vec2(0.f, 1.f));
        tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(0.f, 1.f), cgt::col4(0, 0, 0, 255), cgt::col4(255, 255, 255, 255)));
        _usFusion.p_transferFunction.replaceTF(tf);

        _renderTargetID.setValue("us.fused");

        setEnabled(false);
    }

    void CmBatchGeneration::deinit() {
        _imageWriter.deinit();
        ShdrMgr.dispose(_shader);
        AutoEvaluationPipeline::deinit();
    }

    void CmBatchGeneration::paint() {
        if (p_showFan.getValue()) {
            ImageRepresentationLocal::ScopedRepresentation input(getDataContainer(), _usReader.p_targetImageID.getValue());
            if (input) {
                const cgt::ivec2 inputSize = input->getSize().xy();
                auto vertices = _scanlineConverter.generateLookupVertices(input->getParent());
                _shader->activate();

                _shader->setUniform("_viewMatrix", cgt::mat4::createTranslation(cgt::vec3(-1.f, -1.f, -1.f)) * cgt::mat4::createScale(cgt::vec3(2.f, 2.f, 2.f)));
                _shader->setUniform("_modelMatrix", cgt::mat4::createScale(cgt::vec3(1.f / float(inputSize.x), 1.f / float(inputSize.y), 1.f)));
                glPointSize(3.f);

                glEnable (GL_BLEND);
                glBlendFunc (GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

                std::vector<cgt::vec4> colors(vertices.size(), cgt::vec4(1.f, 0.7f, 0.f, 0.4f));
                FaceGeometry face(vertices, std::vector<cgt::vec3>(), colors);
                face.render(GL_POINTS);
                glDisable(GL_BLEND);

                glPointSize(1.f);
                _shader->deactivate();
            }
        }
    }

    void CmBatchGeneration::onPropertyChanged(const AbstractProperty* p) {
        if (p == &p_showFan)
            setPipelineDirty();
        else 
            AutoEvaluationPipeline::onPropertyChanged(p);
    }

    void CmBatchGeneration::onProcessorInvalidated(AbstractProcessor* processor) {
        if (processor == &_scanlineConverter && p_showFan.getValue()) {
            setPipelineDirty();
        }

        AutoEvaluationPipeline::onProcessorInvalidated(processor);
    }

    void CmBatchGeneration::startBatchProcess() {
        if (p_range.getValue().x > p_range.getValue().y)
            return;

        p_autoExecution.setValue(false);

        getDataContainer().removeData(_confidenceGenerator.p_targetImageID.getValue());
        getDataContainer().removeData(_confidenceGenerator.p_targetImageID.getValue() + "velocities");

        cgt::GLContextScopedLock lock(_canvas);

        cgt::ivec2 originalCanvasSize = _canvasSize.getValue();
        _canvasSize.setValue(_scanlineConverter.p_targetSize.getValue());
        
        for (int i = p_range.getValue().x; i < p_range.getValue().y; ++i) {
            executePass(i);
        }

        _canvasSize.setValue(originalCanvasSize);
    }

    void CmBatchGeneration::executePass(int path) {
        // set up processors:
        std::stringstream ss;
        ss << "export" << std::setfill('0') << std::setw(4) << path << ".bmp";
        std::string fileName = ss.str();

        // read image
        _usReader.p_url.setValue(p_sourcePath.getValue() + "\\" + fileName);
        forceExecuteProcessor(&_usReader);
        forceExecuteProcessor(&_scanlineConverter);
        forceExecuteProcessor(&_confidenceGenerator);
        forceExecuteProcessor(&_usBlurFilter);

        _usFusion.p_transferFunction.setAutoFitWindowToData(false);
        _usFusion.p_transferFunction.getTF()->setIntensityDomain(cgt::vec2(0.f, 1.f));
        {
            // Resampled
            Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(256);
            tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(0.0f, 1.0f), cgt::col4(0, 0, 0, 255), cgt::col4(0, 0, 0, 0)));
            _usFusion.p_confidenceTF.replaceTF(tf);
            _usFusion.p_view.selectById("us");
            forceExecuteProcessor(&_usFusion);
            save(_usFusion.p_targetImageID.getValue(), p_targetPathResampled.getValue() + "\\" + fileName);
        }

        {
            // Confidence Map
            _usFusion.p_view.selectById("cm");
            forceExecuteProcessor(&_usFusion);
            save(_usFusion.p_targetImageID.getValue(), p_targetPathCmCpu.getValue() + "\\" + fileName);
        }

        {
            // Color Overlay mapping
            Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(256);
            tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(0.0f, 0.45f), cgt::col4(0, 0, 0, 224), cgt::col4(0, 0, 0, 0)));
            _usFusion.p_confidenceTF.replaceTF(tf);
            _usFusion.p_hue.setValue(0.15f);
            _usFusion.p_view.selectById("colorOverlay");
            forceExecuteProcessor(&_usFusion);
            save(_usFusion.p_targetImageID.getValue(), p_targetPathColorOverlay.getValue() + "\\" + fileName);
        }

        {
            // LAB mapping
            Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(256);
            tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(0.0f, 0.5f), cgt::col4(0, 0, 0, 224), cgt::col4(0, 0, 0, 0)));
            _usFusion.p_confidenceTF.replaceTF(tf);
            _usFusion.p_hue.setValue(0.23f);
            _usFusion.p_view.selectById("mappingLAB");
            forceExecuteProcessor(&_usFusion);
            save(_usFusion.p_targetImageID.getValue(), p_targetPathColor.getValue() + "\\" + fileName);
        }

        {
            // Fuzziness mapping
            Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(256);
            tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(0.0f, 1.0f), cgt::col4(0, 0, 0, 255), cgt::col4(0, 0, 0, 0)));
            _usFusion.p_confidenceTF.replaceTF(tf);
            _usFusion.p_view.selectById("mappingSharpness");
            forceExecuteProcessor(&_usFusion);
            save(_usFusion.p_targetImageID.getValue(), p_targetPathFuzzy.getValue() + "\\" + fileName);
        }
    }

    void CmBatchGeneration::save(const std::string& dataName, const std::string& fileName) {
        _imageWriter.p_inputImage.setValue(dataName);
        _imageWriter.p_url.setValue(fileName);
        _imageWriter.p_writeDepthImage.setValue(false);
        forceExecuteProcessor(&_imageWriter);
    }

}
