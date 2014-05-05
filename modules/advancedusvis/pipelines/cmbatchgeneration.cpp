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

#include "cmbatchgeneration.h"

#include "tgt/event/keyevent.h"
#include "tgt/filesystem.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"
#include "core/tools/opengljobprocessor.h"
#include "core/tools/simplejobprocessor.h"
#include "core/tools/job.h"

#ifdef CAMPVIS_HAS_MODULE_DEVIL
#include <IL/il.h>
#include <IL/ilu.h>
#endif

#include <sstream>
#include <iomanip>

namespace campvis {

    CmBatchGeneration::CmBatchGeneration(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _usReader(&_canvasSize)
        , _confidenceGenerator()
        , _usBlurFilter(&_canvasSize)
        , _usFusion(&_canvasSize)
        , p_autoExecution("AutoExecution", "Automatic Execution", false)
        , p_sourcePath("SourcePath", "Source Files Path", "", StringProperty::DIRECTORY)
        , p_targetPathColor("TargetPathColor", "Target Path Color Files", "", StringProperty::DIRECTORY)
        , p_targetPathFuzzy("TargetPathFuzzy", "Target Path Fuzzy Files", "", StringProperty::DIRECTORY)
        , p_range("Range", "Files Range", tgt::ivec2(0, 1), tgt::ivec2(0, 0), tgt::ivec2(10000, 10000))
        , p_execute("Execute", "Execute Batch Pipeline")
    {
        addProcessor(&_usReader);
        addProcessor(&_confidenceGenerator);
        addProcessor(&_usFusion);
        addProcessor(&_usBlurFilter);

        addProperty(p_autoExecution);
        addProperty(p_sourcePath);
        addProperty(p_targetPathColor);
        addProperty(p_targetPathFuzzy);
        addProperty(p_range);
        addProperty(p_execute);
    }

    CmBatchGeneration::~CmBatchGeneration() {
    }

    void CmBatchGeneration::init() {
        AutoEvaluationPipeline::init();

        p_sourcePath.setValue("D:\\Medical Data\\US Confidence Vis\\Pasing 13-02-26\\04-02-22-212506_Perez11_20040222_212506_20040222_220332\\gallenblase");
        p_targetPathColor.setValue("D:\\Medical Data\\US Confidence Vis\\Pasing 13-02-26\\04-02-22-212506_Perez11_20040222_212506_20040222_220332\\gallenblase\\color");
        p_targetPathFuzzy.setValue("D:\\Medical Data\\US Confidence Vis\\Pasing 13-02-26\\04-02-22-212506_Perez11_20040222_212506_20040222_220332\\gallenblase\\fuzzy");
        p_range.setValue(tgt::ivec2(0, 1));
        p_execute.s_clicked.connect(this, &CmBatchGeneration::execute);

        _usReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\Pasing 13-02-26\\04-02-22-212506_Perez11_20040222_212506_20040222_220332\\11_niere_re_durch_leber2\\original\\export0000.bmp");
        _usReader.p_targetImageID.setValue("us.image");
        _usReader.p_importType.selectById("localIntensity");
        _usReader.p_targetImageID.addSharedProperty(&_confidenceGenerator.p_sourceImageID);
        _usReader.p_targetImageID.addSharedProperty(&_usFusion.p_usImageId);
        _usReader.p_targetImageID.addSharedProperty(&_usBlurFilter.p_inputImage);

        _confidenceGenerator.p_targetImageID.setValue("confidence.image.generated");
        _confidenceGenerator.p_targetImageID.addSharedProperty(&_usFusion.p_confidenceImageID);
        _confidenceGenerator.p_curvilinear.setValue(true);
        _confidenceGenerator.p_origin.setValue(tgt::vec2(340.f, 540.f));
        _confidenceGenerator.p_angles.setValue(tgt::vec2(4.064f, 5.363f));
        //_confidenceGenerator.p_angles.setValue(tgt::vec2(232.f / 180.f * tgt::PIf, 307.f / 180.f * tgt::PIf));
        //_confidenceGenerator.p_origin.setValue(tgt::vec2(320.f, 35.f));
        //_confidenceGenerator.p_angles.setValue(tgt::vec2(45.f / 180.f * tgt::PIf, 135.f / 180.f * tgt::PIf));
        _confidenceGenerator.p_lengths.setValue(tgt::vec2(116.f, 543.f));

        _usFusion.p_targetImageID.setValue("us.fused");
        _usFusion.p_view.selectById("mappingSharpness");
        _usFusion.p_sliceNumber.setValue(0);

        _usBlurFilter.p_outputImage.setValue("us.blurred");
        _usBlurFilter.p_outputImage.addSharedProperty(&_usFusion.p_blurredImageId);
        _usBlurFilter.p_sigma.setValue(2.f);


        // TODO: replace this hardcoded domain by automatically determined from image min/max values
        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(128, tgt::vec2(0.f, 1.f));
        tf->addGeometry(TFGeometry1D::createQuad(tgt::vec2(0.f, 1.f), tgt::col4(0, 0, 0, 255), tgt::col4(255, 255, 255, 255)));
        _usFusion.p_transferFunction.replaceTF(tf);

        _renderTargetID.setValue("us.fused");
    }

    void CmBatchGeneration::deinit() {
        AutoEvaluationPipeline::deinit();
    }

    void CmBatchGeneration::execute() {
        if (p_range.getValue().x > p_range.getValue().y)
            return;

        p_autoExecution.setValue(false);
        for (int i = p_range.getValue().x; i < p_range.getValue().y; ++i) {
            GLJobProc.enqueueJob(_canvas, makeJobOnHeap(this, &CmBatchGeneration::executePass, i), OpenGLJobProcessor::SerialJob);
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

        executeProcessor(&_usReader, false);

        DataHandle dh = _data->getData(_usReader.p_targetImageID.getValue());
        if (dh.getData() != 0) {
            if (const ImageData* tester = dynamic_cast<const ImageData*>(dh.getData())) {
            	_canvasSize.setValue(tester->getSize().xy());
            }
        }

        executeProcessor(&_confidenceGenerator, false);
        executeProcessor(&_usBlurFilter, false);

        _usFusion.p_view.selectById("mappingLAB");
        executeProcessor(&_usFusion, false);
        save(path, p_targetPathColor.getValue());

//         _usFusion.p_view.selectById("mappingSharpness");
//         executeProcessor(&_usFusion);
//         save(path, p_targetPathFuzzy.getValue());
        
    }

    void CmBatchGeneration::save(int path, const std::string& basePath) {
        // get result
        ScopedTypedData<RenderData> rd(*_data, _usFusion.p_targetImageID.getValue());
        const ImageRepresentationGL* rep = rd->getColorTexture()->getRepresentation<ImageRepresentationGL>(false);
        if (rep != 0) {
#ifdef CAMPVIS_HAS_MODULE_DEVIL
            if (! tgt::FileSystem::dirExists(basePath))
                tgt::FileSystem::createDirectory(basePath);

            std::stringstream sss;
            sss << basePath << "\\" << "export" << std::setfill('0') << std::setw(4) << path << ".bmp";
            std::string filename = sss.str();
            if (tgt::FileSystem::fileExtension(filename).empty()) {
                LERROR("Filename has no extension");
                return;
            }

            // get color buffer content
            GLubyte* colorBuffer = rep->getTexture()->downloadTextureToBuffer(GL_RGBA, GL_UNSIGNED_SHORT);
            tgt::ivec2 size = rep->getSize().xy();

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
