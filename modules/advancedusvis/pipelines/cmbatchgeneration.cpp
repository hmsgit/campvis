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
#include "core/tools/simplejobprocessor.h"
#include "core/tools/job.h"

namespace campvis {

    CmBatchGeneration::CmBatchGeneration()
        : VisualizationPipeline()
        , _usReader(_effectiveRenderTargetSize)
        , _confidenceGenerator()
        , _usBlurFilter()
        , _usFusion(_effectiveRenderTargetSize)
    {
        addProcessor(&_usReader);
        addProcessor(&_confidenceGenerator);
        addProcessor(&_usFusion);
        addProcessor(&_usBlurFilter);
    }

    CmBatchGeneration::~CmBatchGeneration() {
    }

    void CmBatchGeneration::init() {
        VisualizationPipeline::init();

        _usReader.s_validated.connect(this, &CmBatchGeneration::onProcessorValidated);


        //_usReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\CurefabCS\\Stent_Patient_ B-Mode_2013-02-11T14.56.46z\\01_us.mhd");
        //_usReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\01\\BMode_01.mhd");
        //_usReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\UltrasoundBoneData\\SynthesEvaluationUnterschenkel");
        _usReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\transcranial\\us.png");
        _usReader.p_targetImageID.setValue("us.image");
        _usReader.p_importType.selectById("localIntensity");
        _usReader.p_targetImageID.connect(&_confidenceGenerator.p_sourceImageID);
        _usReader.p_targetImageID.connect(&_usFusion.p_usImageId);
        _usReader.p_targetImageID.connect(&_usBlurFilter.p_sourceImageID);

        _confidenceGenerator.p_targetImageID.setValue("confidence.image.generated");
        _confidenceGenerator.p_targetImageID.connect(&_usFusion.p_confidenceImageID);
        _confidenceGenerator.p_curvilinear.setValue(true);
        //_confidenceGenerator.p_origin.setValue(tgt::vec2(320.f, 444s.f));
        //_confidenceGenerator.p_angles.setValue(tgt::vec2(225.f / 180.f * tgt::PIf, 315.f / 180.f * tgt::PIf));
        _confidenceGenerator.p_origin.setValue(tgt::vec2(320.f, 35.f));
        _confidenceGenerator.p_angles.setValue(tgt::vec2(45.f / 180.f * tgt::PIf, 135.f / 180.f * tgt::PIf));
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
/*
        if (!_usReader.getInvalidationLevel().isValid()) {
            SimpleJobProc.enqueueJob(makeJob(this, &CmBatchGeneration::foobar));
        }
        if (!_usDenoiseilter.getInvalidationLevel().isValid()) {
            SimpleJobProc.enqueueJob(makeJob<CmBatchGeneration, AbstractProcessor*>(this, &CmBatchGeneration::executeProcessor, &_usDenoiseilter));
        }
        if (!_confidenceGenerator.getInvalidationLevel().isValid()) {
            SimpleJobProc.enqueueJob(makeJob<CmBatchGeneration, AbstractProcessor*>(this, &CmBatchGeneration::executeProcessor, &_confidenceGenerator));
        }


        for (std::vector<AbstractProcessor*>::iterator it = _processors.begin(); it != _processors.end(); ++it) {
            if (! (*it)->getInvalidationLevel().isValid())
                lockGLContextAndExecuteProcessor(*it);
        }*/
    }

    void CmBatchGeneration::keyEvent(tgt::KeyEvent* e) {
    }

    const std::string CmBatchGeneration::getName() const {
        return "CmBatchGeneration";
    }

    void CmBatchGeneration::onProcessorValidated(AbstractProcessor* processor) {
        if (processor = &_usReader) {

        }
    }

}