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

#include "predicatedemoshoulder.h"

#include "tgt/event/keyevent.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/datastructures/imagedata.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

#include "modules/advancedusvis/processors/pointpredicateraycaster.h"

namespace campvis {

    PredicateDemoShoulder::PredicateDemoShoulder(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _lsp()
        , _imageReader()
        , _labelReader()
        , _confidenceReader()
        , _gaussian(&_canvasSize)
        , _vesselnesFilter(&_canvasSize)
        , _snrFilter(&_canvasSize)
        , _ve(&_canvasSize)
    {
        addProcessor(&_lsp);
        addProcessor(&_imageReader);
        addProcessor(&_labelReader);
        addProcessor(&_confidenceReader);
        addProcessor(&_gaussian);
        addProcessor(&_vesselnesFilter);
        addProcessor(&_snrFilter);
        addProcessor(&_ve);

        addEventListenerToBack(&_ve);
    }

    PredicateDemoShoulder::~PredicateDemoShoulder() {
    }

    void PredicateDemoShoulder::init() {
        AutoEvaluationPipeline::init();
        
        _imageReader.s_validated.connect(this, &PredicateDemoShoulder::onProcessorValidated);

        _ve.p_outputImage.setValue("ve");
        _renderTargetID.setValue("ve");

        _imageReader.p_url.setValue(CAMPVIS_SOURCE_DIR "/modules/advancedusvis/sampledata/Volume_0_cropped.mhd");
        //_imageReader.p_url.setValue("D:/Medical Data/us_carotid_2014-02/christian/christian_bmode_2D_compounded_cropped.mhd");
        _imageReader.p_targetImageID.setValue("reader.output");
        //_imageReader.p_targetImageID.addSharedProperty(&_gaussian.p_inputImage);
        _imageReader.p_targetImageID.addSharedProperty(&_snrFilter.p_inputImage);
        _imageReader.p_targetImageID.addSharedProperty(&_ve.p_inputVolume);
        _imageReader.s_validated.connect(this, &PredicateDemoShoulder::onProcessorValidated);

        _labelReader.p_url.setValue(CAMPVIS_SOURCE_DIR "/modules/advancedusvis/sampledata/layers_0_small_cropped.mhd");
        //_labelReader.p_url.setValue("D:/Medical Data/us_carotid_2014-02/christian/christian_bmode_2D_compounded_cropped_layers.mhd");
        _labelReader.p_targetImageID.setValue("labels");
        _labelReader.p_targetImageID.addSharedProperty(&_ve.p_inputLabels);
        
        _confidenceReader.p_url.setValue(CAMPVIS_SOURCE_DIR "/modules/advancedusvis/sampledata/Volume_0_small_confidence_cropped.mhd");
        _confidenceReader.p_targetImageID.setValue("confidence");
        _confidenceReader.p_targetImageID.addSharedProperty(&_ve.p_inputConfidence);
        _confidenceReader.p_targetImageID.addSharedProperty(&_gaussian.p_inputImage);

        _snrFilter.p_outputImage.setValue("snr");
        _snrFilter.p_outputImage.addSharedProperty(&_ve.p_inputSnr);

        _gaussian.p_sigma.setValue(5.f);
        _gaussian.p_outputImage.addSharedProperty(&_vesselnesFilter.p_inputImage);

        _vesselnesFilter.p_outputImage.setValue("vesselness");
        _vesselnesFilter.p_outputImage.addSharedProperty(&_ve.p_inputVesselness);

        Geometry1DTransferFunction* dvrTF = new Geometry1DTransferFunction(128, tgt::vec2(0.05f, .8f));
        dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(0.f, 1.f), tgt::col4(0, 0, 0, 0), tgt::col4(255, 255, 255, 255)));
        static_cast<TransferFunctionProperty*>(_ve.getNestedProperty("VolumeRendererProperties::RaycasterProps::TransferFunction"))->setAutoFitWindowToData(false);
        static_cast<TransferFunctionProperty*>(_ve.getNestedProperty("VolumeRendererProperties::RaycasterProps::TransferFunction"))->replaceTF(dvrTF);
        static_cast<FloatProperty*>(_ve.getNestedProperty("VolumeRendererProperties::RaycasterProps::GradientLod"))->setValue(0.5f);
        static_cast<FloatProperty*>(_ve.getNestedProperty("VolumeRendererProperties::RaycasterProps::SamplingRate"))->setValue(1.f);

//         static_cast<Vec4Property*>(_ve.getNestedProperty("backgroundColor1"))->setValue(tgt::vec4(1.f));
//         static_cast<Vec4Property*>(_ve.getNestedProperty("backgroundColor2"))->setValue(tgt::vec4(1.f));

        _canvasSize.s_changed.connect<PredicateDemoShoulder>(this, &PredicateDemoShoulder::onRenderTargetSizeChanged);

        _canvasSize.setVisible(false);
        _renderTargetID.setVisible(false);

        // initialize predicates with default config
        PointPredicateHistogramProperty* php = &_ve.p_histogram;
        if (php != nullptr) {
            PointPredicateHistogram* histogram = php->getPredicateHistogram();

            AbstractPointPredicate* vpToAdd = 0;
            vpToAdd = new RangePointPredicate("gradientMagnitude", "GradMag", "Gradient Magnitude Range");
            static_cast<RangePointPredicate*>(vpToAdd)->p_range.setValue(tgt::vec2(.15f, 1.f));
            histogram->addPredicate(vpToAdd);

            vpToAdd = new RangePointPredicate("intensity", "Intensity", "Intensity Range");
            static_cast<RangePointPredicate*>(vpToAdd)->p_range.setValue(tgt::vec2(.05f, 1.f));
            histogram->addPredicate(vpToAdd);

            vpToAdd = new RangePointPredicate("snr", "SNR", "SNR Range");
            static_cast<RangePointPredicate*>(vpToAdd)->p_range.setMaxValue(tgt::vec2(10.f, 10.f));
            static_cast<RangePointPredicate*>(vpToAdd)->p_range.setValue(tgt::vec2(1.25f, 10.f));
            vpToAdd->p_intensityHack.setValue(.125f);
            histogram->addPredicate(vpToAdd);

            vpToAdd = new LabelBitPointPredicate("label", "Skin", "Skin Layer");
            static_cast<LabelBitPointPredicate*>(vpToAdd)->p_bit.setValue(0);
            histogram->addPredicate(vpToAdd);

            vpToAdd = new LabelBitPointPredicate("label", "Muscle", "Muscle Layer");
            static_cast<LabelBitPointPredicate*>(vpToAdd)->p_bit.setValue(1);
            histogram->addPredicate(vpToAdd);

            vpToAdd = new LabelBitPointPredicate("label", "Bone", "Bone Layer");
            static_cast<LabelBitPointPredicate*>(vpToAdd)->p_bit.setValue(3);
            histogram->addPredicate(vpToAdd);

            histogram->resetPredicates();
            addProperty(*php);
        }
    }

    void PredicateDemoShoulder::deinit() {
        AutoEvaluationPipeline::deinit();
    }

    void PredicateDemoShoulder::onRenderTargetSizeChanged(const AbstractProperty* prop) {
    }

    void PredicateDemoShoulder::onProcessorValidated(AbstractProcessor* processor) {
        if (processor == &_imageReader) {
            if (IVec2Property* tester = dynamic_cast<IVec2Property*>(_ve.getNestedProperty("VolumeRendererProperties::PGGProps::clipX"))) {
                tester->setValue(tgt::ivec2(42, 210));
            }
        }
    }
}
