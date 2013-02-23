// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#include "cmbatchgeneration.h"

#include "tgt/event/keyevent.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"
#include "core/datastructures/imagerepresentationrendertarget.h"
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

    CmBatchGeneration::CmBatchGeneration()
        : VisualizationPipeline()
        , _usReader(_effectiveRenderTargetSize)
        , _confidenceGenerator()
        , _usBlurFilter()
        , _usFusion(_effectiveRenderTargetSize)
        , p_sourcePath("SourcePath", "Source Files Path", "")
        , p_targetPath("TargetPath", "Target Files Path", "")
        , p_range("Range", "Files Range", tgt::ivec2(0, 1), tgt::ivec2(0, 0), tgt::ivec2(10000, 10000))
        , p_execute("Execute", "Execute Batch Pipeline")
    {
        addProcessor(&_usReader);
        addProcessor(&_confidenceGenerator);
        addProcessor(&_usFusion);
        addProcessor(&_usBlurFilter);

        addProperty(&p_sourcePath);
        addProperty(&p_targetPath);
        addProperty(&p_range);
        addProperty(&p_execute);
    }

    CmBatchGeneration::~CmBatchGeneration() {
    }

    void CmBatchGeneration::init() {
        VisualizationPipeline::init();

        p_sourcePath.setValue("D:\\Medical Data\\US Confidence Vis\\transcranial\\bl01");
        p_targetPath.setValue("D:\\Medical Data\\US Confidence Vis\\transcranial\\bl01cm");
        p_range.setValue(tgt::ivec2(0, 1));
        p_execute.s_clicked.connect(this, &CmBatchGeneration::execute);

        _usReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\transcranial\\us.png");
        _usReader.p_targetImageID.setValue("us.image");
        _usReader.p_importType.selectById("localIntensity");
        _usReader.p_targetImageID.connect(&_confidenceGenerator.p_sourceImageID);
        _usReader.p_targetImageID.connect(&_usFusion.p_usImageId);
        _usReader.p_targetImageID.connect(&_usBlurFilter.p_sourceImageID);

        _confidenceGenerator.p_targetImageID.setValue("confidence.image.generated");
        _confidenceGenerator.p_targetImageID.connect(&_usFusion.p_confidenceImageID);
        _confidenceGenerator.p_curvilinear.setValue(true);
        _confidenceGenerator.p_origin.setValue(tgt::vec2(320.f, 444.f));
        _confidenceGenerator.p_angles.setValue(tgt::vec2(225.f / 180.f * tgt::PIf, 315.f / 180.f * tgt::PIf));
        //_confidenceGenerator.p_origin.setValue(tgt::vec2(320.f, 35.f));
        //_confidenceGenerator.p_angles.setValue(tgt::vec2(45.f / 180.f * tgt::PIf, 135.f / 180.f * tgt::PIf));
        _confidenceGenerator.p_lengths.setValue(tgt::vec2(0.f, 410.f));

        _usFusion.p_targetImageID.setValue("us.fused");
        _usFusion.p_view.selectById("mappingSharpness");
        _usFusion.p_sliceNumber.setValue(0);

        _usBlurFilter.p_targetImageID.setValue("us.blurred");
        _usBlurFilter.p_targetImageID.connect(&_usFusion.p_blurredImageId);
        _usBlurFilter.p_filterMode.selectById("gauss");
        _usBlurFilter.p_sigma.setValue(4.f);


        // TODO: replace this hardcoded domain by automatically determined from image min/max values
        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(128, tgt::vec2(0.f, 1.f));
        tf->addGeometry(TFGeometry1D::createQuad(tgt::vec2(0.f, 1.f), tgt::col4(0, 0, 0, 255), tgt::col4(255, 255, 255, 255)));
        _usFusion.p_transferFunction.replaceTF(tf);

        _renderTargetID.setValue("us.fused");
    }

    void CmBatchGeneration::deinit() {
        VisualizationPipeline::deinit();
    }

    void CmBatchGeneration::execute() {
        if (p_range.getValue().x > p_range.getValue().y)
            return;

        for (int i = p_range.getValue().x; i < p_range.getValue().y; ++i) {
            GLJobProc.enqueueJob(_canvas, makeJobOnHeap(this, &CmBatchGeneration::executePass, i), OpenGLJobProcessor::SerialJob);
        }
    }

    const std::string CmBatchGeneration::getName() const {
        return "CmBatchGeneration";
    }

    void CmBatchGeneration::onProcessorInvalidated(AbstractProcessor* processor) {
        // do nothing here - we do our own evaluation
    }

    void CmBatchGeneration::executePass(int path) {
        std::stringstream ss;

        // set up processors:
        ss << p_sourcePath.getValue() << "\\" << "export" << std::setfill('0') << std::setw(4) << path << ".tga";
        _usReader.p_url.setValue(ss.str());

        executeProcessor(&_usReader);

        DataHandle dh = _data.getData(_usReader.p_targetImageID.getValue());
        if (dh.getData() != 0) {
            if (const ImageData* tester = dynamic_cast<const ImageData*>(dh.getData())) {
            	_canvasSize.setValue(tester->getSize().xy());
            }
        }

        executeProcessor(&_confidenceGenerator);
        executeProcessor(&_usBlurFilter);
        executeProcessor(&_usFusion);

        // get result
        ImageRepresentationRenderTarget::ScopedRepresentation repRT(_data, _usFusion.p_targetImageID.getValue());
        if (repRT != 0) {
#ifdef CAMPVIS_HAS_MODULE_DEVIL
            std::stringstream sss;
            sss << p_targetPath.getValue() << "\\" << "export" << std::setfill('0') << std::setw(4) << path << ".png";
            std::string filename = sss.str();
            if (tgt::FileSystem::fileExtension(filename).empty()) {
                LERROR("Filename has no extension");
                return;
            }

            // get color buffer content
            GLubyte* colorBuffer = repRT->getColorTexture()->downloadTextureToBuffer(GL_RGBA, GL_UNSIGNED_SHORT);
            tgt::ivec2 size = repRT->getSize().xy();

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