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

#include "predicatedemoshoulder.h"

#include "cgt/event/keyevent.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/datastructures/imagedata.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

#include "modules/advancedusvis/processors/pointpredicateraycaster.h"

namespace campvis {

    PredicateDemoShoulder::PredicateDemoShoulder(DataContainer& dc)
        : AutoEvaluationPipeline(dc, getId())
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

        Geometry1DTransferFunction* dvrTF = new Geometry1DTransferFunction(128, cgt::vec2(0.05f, .8f));
        dvrTF->addGeometry(TFGeometry1D::createQuad(cgt::vec2(0.f, 1.f), cgt::col4(0, 0, 0, 0), cgt::col4(255, 255, 255, 255)));
        static_cast<TransferFunctionProperty*>(_ve.getNestedProperty("VolumeRendererProperties::RaycasterProps::TransferFunction"))->setAutoFitWindowToData(false);
        static_cast<TransferFunctionProperty*>(_ve.getNestedProperty("VolumeRendererProperties::RaycasterProps::TransferFunction"))->replaceTF(dvrTF);
        static_cast<FloatProperty*>(_ve.getNestedProperty("VolumeRendererProperties::RaycasterProps::GradientLod"))->setValue(0.5f);
        static_cast<FloatProperty*>(_ve.getNestedProperty("VolumeRendererProperties::RaycasterProps::SamplingRate"))->setValue(1.f);

//         static_cast<Vec4Property*>(_ve.getNestedProperty("backgroundColor1"))->setValue(cgt::vec4(1.f));
//         static_cast<Vec4Property*>(_ve.getNestedProperty("backgroundColor2"))->setValue(cgt::vec4(1.f));

        _canvasSize.setVisible(false);
        _renderTargetID.setVisible(false);

        // initialize predicates with default config
        PointPredicateHistogramProperty* php = &_ve.p_histogram;
        if (php != nullptr) {
            PointPredicateHistogram* histogram = php->getPredicateHistogram();

            AbstractPointPredicate* vpToAdd = 0;
            vpToAdd = new RangePointPredicate("gradientMagnitude", "GradMag", "Gradient Magnitude Range");
            static_cast<RangePointPredicate*>(vpToAdd)->p_range.setValue(cgt::vec2(.15f, 1.f));
            histogram->addPredicate(vpToAdd);

            vpToAdd = new RangePointPredicate("intensity", "Intensity", "Intensity Range");
            static_cast<RangePointPredicate*>(vpToAdd)->p_range.setValue(cgt::vec2(.05f, 1.f));
            histogram->addPredicate(vpToAdd);

            vpToAdd = new RangePointPredicate("snr", "SNR", "SNR Range");
            static_cast<RangePointPredicate*>(vpToAdd)->p_range.setMaxValue(cgt::vec2(10.f, 10.f));
            static_cast<RangePointPredicate*>(vpToAdd)->p_range.setValue(cgt::vec2(1.25f, 10.f));
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
                tester->setValue(cgt::ivec2(42, 210));
            }
        }
    }
}
