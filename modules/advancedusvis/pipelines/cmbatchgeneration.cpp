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
#include "cgt/event/keyevent.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"

#ifdef CAMPVIS_HAS_MODULE_DEVIL
#include <IL/il.h>
#include <IL/ilu.h>
#endif

#include <sstream>
#include <iomanip>

namespace campvis {

    CmBatchGeneration::CmBatchGeneration(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _usReader()
        , _confidenceGenerator()
        , _usBlurFilter(&_canvasSize)
        , _usFusion(&_canvasSize)
        , p_autoExecution("AutoExecution", "Automatic Execution", false)
        , p_sourcePath("SourcePath", "Source Files Path", "", StringProperty::DIRECTORY)
        , p_targetPathCm("TargetPathCm", "Target Path Confidence Map Files", "", StringProperty::DIRECTORY)
        , p_targetPathColorOverlay("TargetPathColorOverlay", "Target Path Color Overlay Files", "", StringProperty::DIRECTORY)
        , p_targetPathColor("TargetPathColor", "Target Path Color Files", "", StringProperty::DIRECTORY)
        , p_targetPathFuzzy("TargetPathFuzzy", "Target Path Fuzzy Files", "", StringProperty::DIRECTORY)
        , p_range("Range", "Files Range", cgt::ivec2(0, 1), cgt::ivec2(0, 0), cgt::ivec2(10000, 10000))
        , p_execute("Execute", "Execute Batch Pipeline")
    {
        addProcessor(&_usReader);
        addProcessor(&_confidenceGenerator);
        addProcessor(&_usFusion);
        addProcessor(&_usBlurFilter);

        addProperty(p_autoExecution);
        addProperty(p_sourcePath);
        addProperty(p_targetPathCm);
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

        p_sourcePath.setValue("D:\\cm_stuff\\original");
        p_targetPathCm.setValue("D:\\cm_stuff\\cm");
        p_targetPathColorOverlay.setValue("D:\\cm_stuff\\colorOverlay");
        p_targetPathColor.setValue("D:\\cm_stuff\\color");
        p_targetPathFuzzy.setValue("D:\\cm_stuff\\fuzzy");
        p_range.setValue(cgt::ivec2(0, 1));
        p_execute.s_clicked.connect(this, &CmBatchGeneration::execute);

        _usReader.p_url.setValue("D:\\cm_stuff\\original\\export0000.bmp");
        _usReader.p_targetImageID.setValue("us.image");
        _usReader.p_importType.selectById("localIntensity");
        _usReader.p_targetImageID.addSharedProperty(&_confidenceGenerator.p_sourceImageID);
        _usReader.p_targetImageID.addSharedProperty(&_usFusion.p_usImageId);
        _usReader.p_targetImageID.addSharedProperty(&_usBlurFilter.p_inputImage);

        _confidenceGenerator.p_targetImageID.setValue("confidence.image.generated");
        _confidenceGenerator.p_targetImageID.addSharedProperty(&_usFusion.p_confidenceImageID);
        _confidenceGenerator.p_curvilinear.setValue(true);
        _confidenceGenerator.p_origin.setValue(cgt::vec2(340.f, 540.f));
        _confidenceGenerator.p_angles.setValue(cgt::vec2(4.064f, 5.363f));
        //_confidenceGenerator.p_angles.setValue(cgt::vec2(232.f / 180.f * cgt::PIf, 307.f / 180.f * cgt::PIf));
        //_confidenceGenerator.p_origin.setValue(cgt::vec2(320.f, 35.f));
        //_confidenceGenerator.p_angles.setValue(cgt::vec2(45.f / 180.f * cgt::PIf, 135.f / 180.f * cgt::PIf));
        _confidenceGenerator.p_lengths.setValue(cgt::vec2(116.f, 543.f));
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
        AutoEvaluationPipeline::deinit();
    }

    void CmBatchGeneration::execute() {
        if (p_range.getValue().x > p_range.getValue().y)
            return;

        p_autoExecution.setValue(false);

        cgt::GLContextScopedLock lock(_canvas);

        getDataContainer().removeData(_confidenceGenerator.p_targetImageID.getValue());
        getDataContainer().removeData(_confidenceGenerator.p_targetImageID.getValue() + "velocities");

        for (int i = p_range.getValue().x; i < p_range.getValue().y; ++i) {
            executePass(i);
        }
    }

    void CmBatchGeneration::onProcessorInvalidated(AbstractProcessor* processor) {
        if (p_autoExecution.getValue())
            AutoEvaluationPipeline::onProcessorInvalidated(processor);
    }

    void CmBatchGeneration::executePass(int path) {
        std::stringstream ss;

        // set up processors:
        ss << p_sourcePath.getValue() << "\\" << "export" << std::setfill('0') << std::setw(4) << path << ".bmp";
        _usReader.p_url.setValue(ss.str());

        forceExecuteProcessor(&_usReader);

        DataHandle dh = _data->getData(_usReader.p_targetImageID.getValue());
        if (dh.getData() != 0) {
            if (const ImageData* tester = dynamic_cast<const ImageData*>(dh.getData())) {
            	_canvasSize.setValue(tester->getSize().xy());
            }
        }

        forceExecuteProcessor(&_confidenceGenerator);
        forceExecuteProcessor(&_usBlurFilter);
        _usFusion.p_transferFunction.setAutoFitWindowToData(false);
        _usFusion.p_transferFunction.getTF()->setIntensityDomain(cgt::vec2(0.f, 1.f));

        {
            // Confidence Map
            Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(256);
            tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(0.0f, 1.0f), cgt::col4(0, 0, 0, 255), cgt::col4(0, 0, 0, 0)));
            _usFusion.p_confidenceTF.replaceTF(tf);
            _usFusion.p_view.selectById("cm");
            forceExecuteProcessor(&_usFusion);
            save(path, p_targetPathCm.getValue());
        }

        {
            // Color Overlay mapping
            Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(256);
            tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(0.0f, 0.45f), cgt::col4(0, 0, 0, 224), cgt::col4(0, 0, 0, 0)));
            _usFusion.p_confidenceTF.replaceTF(tf);
            _usFusion.p_hue.setValue(0.15f);
            _usFusion.p_view.selectById("colorOverlay");
            forceExecuteProcessor(&_usFusion);
            save(path, p_targetPathColorOverlay.getValue());
        }

        {
            // LAB mapping
            Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(256);
            tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(0.0f, 0.5f), cgt::col4(0, 0, 0, 224), cgt::col4(0, 0, 0, 0)));
            _usFusion.p_confidenceTF.replaceTF(tf);
            _usFusion.p_hue.setValue(0.23f);
            _usFusion.p_view.selectById("mappingLAB");
            forceExecuteProcessor(&_usFusion);
            save(path, p_targetPathColor.getValue());
        }

        {
            // Fuzziness mapping
            Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(256);
            tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(0.0f, 1.0f), cgt::col4(0, 0, 0, 255), cgt::col4(0, 0, 0, 0)));
            _usFusion.p_confidenceTF.replaceTF(tf);
            _usFusion.p_view.selectById("mappingSharpness");
            forceExecuteProcessor(&_usFusion);
            save(path, p_targetPathFuzzy.getValue());
        }
        
    }

    void CmBatchGeneration::save(int path, const std::string& basePath) {
        // get result
        ScopedTypedData<RenderData> rd(*_data, _usFusion.p_targetImageID.getValue());
        const ImageRepresentationGL* rep = rd->getColorTexture()->getRepresentation<ImageRepresentationGL>(false);
        if (rep != 0) {
#ifdef CAMPVIS_HAS_MODULE_DEVIL
            if (! cgt::FileSystem::dirExists(basePath))
                cgt::FileSystem::createDirectory(basePath);

            std::stringstream sss;
            sss << basePath << "\\" << "export" << std::setfill('0') << std::setw(4) << path << ".bmp";
            std::string filename = sss.str();
            if (cgt::FileSystem::fileExtension(filename).empty()) {
                LERROR("Filename has no extension");
                return;
            }

            // get color buffer content
            GLubyte* colorBuffer = rep->getTexture()->downloadTextureToBuffer(GL_RGBA, GL_UNSIGNED_SHORT);
            cgt::ivec2 size = rep->getSize().xy();

            // create Devil image from image data and write it to file
            ILuint img;
            ilGenImages(1, &img);
            ilBindImage(img);

            // put pixels into IL-Image
            ilTexImage(size.x, size.y, 1, 4, IL_RGBA, IL_UNSIGNED_SHORT, colorBuffer);
            ilEnable(IL_FILE_OVERWRITE);
            ilResetWrite();
            ILboolean success = ilSaveImage(filename.c_str());
            ilDeleteImages(1, &img);

            delete[] colorBuffer;

            if (!success) {
                LERROR("Could not save image to file: " << ilGetError());
            }
#else
            return;
#endif
        }
    }

}
