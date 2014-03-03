// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "nloptregistration.h"

#include "tgt/event/keyevent.h"
#include "tgt/openglgarbagecollector.h"
#include "tgt/painter.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"
#include "core/datastructures/renderdata.h"
#include "core/tools/glreduction.h"
#include "core/tools/job.h"
#include "core/tools/opengljobprocessor.h"

namespace campvis {
    static const GenericOption<nlopt::algorithm> optimizers[3] = {
        GenericOption<nlopt::algorithm>("cobyla", "COBYLA", nlopt::LN_COBYLA),
        GenericOption<nlopt::algorithm>("newuoa", "NEWUOA", nlopt::LN_NEWUOA),
        GenericOption<nlopt::algorithm>("neldermead", "Nelder-Mead Simplex", nlopt::LN_NELDERMEAD)
    };

    NloptRegistration::NloptRegistration(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , p_optimizer("Optimizer", "Optimizer", optimizers, 3)
        , p_liveUpdate("LiveUpdate", "Live Update of Difference Image", false)
        , p_performOptimization("PerformOptimization", "Perform Optimization", AbstractProcessor::INVALID_RESULT)
        , p_forceStop("Force Stop", "Force Stop", AbstractProcessor::VALID)
        , p_translationStepSize("TranslationStepSize", "Initial Step Size Translation", 8.f, .1f, 100.f)
        , p_rotationStepSize("RotationStepSize", "Initial Step Size Rotation", .5f, .01f, tgt::PIf)
        , _referenceReader()
        , _movingReader()
        , _sm()
        , _ve(&_canvasSize)
        , _opt(0)
    {
        addProcessor(&_referenceReader);
        addProcessor(&_movingReader);
        addProcessor(&_sm);
        addProcessor(&_ve);

        addProperty(&p_optimizer);
        addProperty(&p_liveUpdate);
        addProperty(&p_performOptimization);
        addProperty(&p_forceStop);
        addProperty(&p_translationStepSize);
        addProperty(&p_rotationStepSize);

        p_performOptimization.s_clicked.connect(this, &NloptRegistration::onPerformOptimizationClicked);
        p_forceStop.s_clicked.connect(this, &NloptRegistration::forceStop);

        addEventListenerToBack(&_ve);
    }

    NloptRegistration::~NloptRegistration() {
    }

    void NloptRegistration::init() {
        AutoEvaluationPipeline::init();

        _referenceReader.p_url.setValue("D:/Medical Data/SCR/Data/RegSweeps_phantom_cropped/-1S1median/Volume_2.mhd");
        _referenceReader.p_targetImageID.setValue("Reference Image");
        _referenceReader.p_targetImageID.addSharedProperty(&_sm.p_referenceId);

        _movingReader.p_url.setValue("D:/Medical Data/SCR/Data/RegSweeps_phantom_cropped/-1S1median/Volume_3.mhd");
        _movingReader.p_targetImageID.setValue("Moving Image");
        _movingReader.p_targetImageID.addSharedProperty(&_sm.p_movingId);

        _sm.p_differenceImageId.addSharedProperty(&_ve.p_inputVolume);
        _sm.p_metric.selectById("NCC");

        _ve.p_outputImage.setValue("volumeexplorer");
        _renderTargetID.setValue("volumeexplorer");

        Geometry1DTransferFunction* dvrTF = new Geometry1DTransferFunction(128, tgt::vec2(-1.f, 1.f));
        dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(0.f, .5f), tgt::col4(0, 0, 255, 255), tgt::col4(255, 255, 255, 0)));
        dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.5f, 1.f), tgt::col4(255, 255, 255, 0), tgt::col4(255, 0, 0, 255)));
        MetaProperty* mp = static_cast<MetaProperty*>(_ve.getProperty("SliceExtractorProperties"));
        static_cast<TransferFunctionProperty*>(mp->getProperty("transferFunction"))->replaceTF(dvrTF);
        static_cast<TransferFunctionProperty*>(mp->getProperty("transferFunction"))->setAutoFitWindowToData(false);
    }

    void NloptRegistration::deinit() {
        delete _opt;
        _opt = 0;

        AutoEvaluationPipeline::deinit();
    }

    void NloptRegistration::onProcessorValidated(AbstractProcessor* processor) {

    }

    void NloptRegistration::onPerformOptimizationClicked() {
        // Evaluation of the similarity measure needs an OpenGL context, so we need to create an OpenGL job for this.
        GLJobProc.enqueueJob(_canvas, makeJobOnHeap(this, &NloptRegistration::performOptimization), OpenGLJobProcessor::SerialJob);
    }

    void NloptRegistration::performOptimization() {
        ImageRepresentationGL::ScopedRepresentation referenceImage(getDataContainer(), _sm.p_referenceId.getValue());
        ImageRepresentationGL::ScopedRepresentation movingImage(getDataContainer(), _sm.p_movingId.getValue());

        if (_opt != 0) {
            LWARNING("Optimization is already running...");
            return;
        }

        MyFuncData_t mfd = { this, referenceImage, movingImage, 0 };

        _opt = new nlopt::opt(p_optimizer.getOptionValue(), 6);
        if (_sm.p_metric.getOptionValue() == "NCC" || _sm.p_metric.getOptionValue() == "SNR") {
            _opt->set_max_objective(&NloptRegistration::optimizerFunc, &mfd);
        }
        else {
            _opt->set_min_objective(&NloptRegistration::optimizerFunc, &mfd);
        }
        _opt->set_xtol_rel(1e-4);

        std::vector<double> x(6);
        x[0] = _sm.p_translation.getValue().x;
        x[1] = _sm.p_translation.getValue().y;
        x[2] = _sm.p_translation.getValue().z;
        x[3] = _sm.p_rotation.getValue().x;
        x[4] = _sm.p_rotation.getValue().y;
        x[5] = _sm.p_rotation.getValue().z;

        std::vector<double> stepSize(6);
        stepSize[0] = p_translationStepSize.getValue();
        stepSize[1] = p_translationStepSize.getValue();
        stepSize[2] = p_translationStepSize.getValue();
        stepSize[3] = p_rotationStepSize.getValue();
        stepSize[4] = p_rotationStepSize.getValue();
        stepSize[5] = p_rotationStepSize.getValue();
        _opt->set_initial_step(stepSize);

        nlopt::result result = nlopt::SUCCESS;
        try {
            double minf;
            result = _opt->optimize(x, minf);
        }
        catch (std::exception& e) {
            LERROR("Excpetion during optimization: " << e.what());
        }

        if (result >= nlopt::SUCCESS || result <= nlopt::ROUNDOFF_LIMITED) {
            LDEBUG("Optimization successful, took " << mfd._count << " steps.");
            tgt::vec3 t(x[0], x[1], x[2]);
            tgt::vec3 r(x[3], x[4], x[5]);
            _sm.p_translation.setValue(t);
            _sm.p_rotation.setValue(r);

            // compute difference image and render difference volume
            _sm.generateDifferenceImage(_data, referenceImage, movingImage, t, r);
            _ve.process(getDataContainer());
        }

        delete _opt;
        _opt = 0;
    }

    double NloptRegistration::optimizerFunc(const std::vector<double>& x, std::vector<double>& grad, void* my_func_data) {
        tgtAssert(x.size() == 6, "Must have 6 values in x.");
        tgtAssert(my_func_data != 0, "my_func_data must not be 0");

        MyFuncData_t* mfd = static_cast<MyFuncData_t*>(my_func_data);
        ++mfd->_count;
        tgt::vec3 translation(x[0], x[1], x[2]);
        tgt::vec3 rotation(x[3], x[4], x[5]);
        float similarity = mfd->_object->_sm.computeSimilarity(mfd->_reference, mfd->_moving, translation, rotation);
        LDEBUG(translation << rotation << " : " << similarity);

        // perform interactive update if wished
        if (mfd->_object->p_liveUpdate.getValue()) {
            // compute difference image
            mfd->_object->_sm.generateDifferenceImage(mfd->_object->_data, mfd->_reference, mfd->_moving, translation, rotation);

            // render difference volume
            mfd->_object->_ve.process(mfd->_object->getDataContainer());

            // update canvas
            mfd->_object->_canvas->getPainter()->paint();
        }

        // clean up unused GL textures.
        GLGC.deleteGarbage();

        return similarity;
    }

    void NloptRegistration::forceStop() {
        if (_opt != 0)
            _opt->force_stop();
    }


}
