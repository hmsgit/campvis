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

#ifdef CAMPVIS_HAS_MODULE_DEVIL

#include "ssimdemo.h"

#include "cgt/glcontextmanager.h"
#include "cgt/shadermanager.h"
#include "core/datastructures/imagerepresentationgl.h"

#include <iomanip>
#include <fstream>
#include <sstream>

namespace campvis {

    SsimDemo::SsimDemo(DataContainer& dc)
        : AutoEvaluationPipeline(dc, getId())
        , _imageReader1()
        , _imageReader2()
        , _ssim(&_canvasSize)
        , _fanRenderer(&_canvasSize)
        , _sumReduction(nullptr)
        , _minReduction(nullptr)
        , _currentlyBatchProcessing(false)
        , p_sourcePath1("SourcePath1", "Source Path 1", "Z:/cm_stuff/cmCuda_512_1000", StringProperty::DIRECTORY)
        , p_sourcePath2("SourcePath2", "Source Path 2", "Z:/cm_stuff/cmCuda_128_1000", StringProperty::DIRECTORY)
        , p_range("Range", "range", cgt::ivec2(0, 399), cgt::ivec2(0), cgt::ivec2(1000))
        , p_execute("Excecute", "Execute Batch Process")
    {
        addProcessor(&_imageReader1);
        addProcessor(&_imageReader2);
        addProcessor(&_ssim);
        addProcessor(&_fanRenderer);

        addProperty(p_sourcePath1);
        addProperty(p_sourcePath2);
        addProperty(p_range);
        addProperty(p_execute);
        p_execute.s_clicked.connect(this, &SsimDemo::executeBatchProcess);
    }

    SsimDemo::~SsimDemo() {
    }

    void SsimDemo::init() {
        AutoEvaluationPipeline::init();

        _sumReduction = new GlReduction(GlReduction::PLUS);
        _minReduction = new GlReduction(GlReduction::MIN);

        _renderTargetID.setValue("us.fan");

        _imageReader1.p_url.setValue("Z:/cm_stuff/cmCuda_512_1000/export0026.bmp");
        _imageReader1.p_importType.setValue(1);
        _imageReader1.p_targetImageID.setValue("reader1.output");

        _imageReader2.p_url.setValue("Z:/cm_stuff/cmCuda_128_1000/export0026.bmp");
        _imageReader2.p_importType.setValue(1);
        _imageReader2.p_targetImageID.setValue("reader2.output");

        _ssim.p_inputImage1.setValue("reader1.output");
        _ssim.p_inputImage2.setValue("reader2.output");
        _ssim.s_validated.connect(this, &SsimDemo::onProcessorValidated);
        _ssim.p_outputImage.setValue("ssim");

        _fanRenderer.p_inputImage.setValue("ssim");
        _fanRenderer.p_renderTargetID.setValue("us.fan");

    }

    void SsimDemo::deinit() {
        delete _minReduction;
        delete _sumReduction;
        AutoEvaluationPipeline::deinit();
    }

    void SsimDemo::onProcessorValidated(AbstractProcessor* p) {
        if (_currentlyBatchProcessing)
            return;

        if (p == &_ssim) {
            ImageRepresentationGL::ScopedRepresentation ssim(getDataContainer(), _ssim.p_outputImage.getValue());
            if (ssim) {
                cgt::GLContextScopedLock lock(this->_canvas);
                auto sums = _sumReduction->reduce(ssim->getTexture());
                auto mins = _minReduction->reduce(ssim->getTexture());

                LINFO("Structured Similarity, Average: " << (sums[0] / ssim->getNumElements()) << ", Minimum: " << mins[0]);
            }
        }
    }

    void SsimDemo::executeBatchProcess() {
        if (p_range.getValue().x > p_range.getValue().y)
            return;

        _currentlyBatchProcessing = true;
        cgt::GLContextScopedLock lock(_canvas);

        float sumsum = 0.f;
        float minmin = 1.f;

        std::string foo = p_sourcePath1.getValue().substr(p_sourcePath1.getValue().find_last_of("/\\") + 1);
        std::string bar = p_sourcePath2.getValue().substr(p_sourcePath2.getValue().find_last_of("/\\") + 1);

        LINFO("Comparing " << p_sourcePath1.getValue() << " to " << p_sourcePath2.getValue() << ":");

        std::ofstream csvFile("C:\\temp\\" + foo + "---" + bar + ".csv");
        csvFile << "index, AvgSSIM, MinSSIM\n";

        for (int i = p_range.getValue().x; i < p_range.getValue().y; ++i) {
            // set up processors:
            std::stringstream ss;
            ss << "export" << std::setfill('0') << std::setw(4) << i << ".bmp";
            std::string fileName = ss.str();

            // read image
            _imageReader1.p_url.setValue(p_sourcePath1.getValue() + "\\" + fileName);
            _imageReader2.p_url.setValue(p_sourcePath2.getValue() + "\\" + fileName);
            forceExecuteProcessor(&_imageReader1);
            forceExecuteProcessor(&_imageReader2);
            forceExecuteProcessor(&_ssim);

            ImageRepresentationGL::ScopedRepresentation ssim(getDataContainer(), _ssim.p_outputImage.getValue());
            if (ssim) {
                auto sums = _sumReduction->reduce(ssim->getTexture());
                auto mins = _minReduction->reduce(ssim->getTexture());

                sumsum += sums[0] / ssim->getNumElements();
                minmin = std::min(minmin, mins[0]);

                csvFile << i << ", " << (sums[0] / ssim->getNumElements()) << ", " << mins[0] << "\n";
            }

            if (i % 100 == 0) 
                LINFO(i);
        }

        LINFO("Structured Similarity, Averaged average: " << (sumsum / (p_range.getValue().y - p_range.getValue().x)) << ", Minimum of the Minima: " << minmin);
        _currentlyBatchProcessing = false;
    }

}

#endif // CAMPVIS_HAS_MODULE_DEVIL
