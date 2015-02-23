
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

#include "volumerenderer.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"

#include "core/classification/simpletransferfunction.h"

#include "cgt/opengljobprocessor.h"

namespace campvis {
    const std::string VolumeRenderer::loggerCat_ = "CAMPVis.modules.vis.VolumeRenderer";

    //static const std::string* raycastingProcessorName = &RaycasterFactory::getRef().getRegisteredRaycasters()[0];

    VolumeRenderer::VolumeRenderer(IVec2Property* viewportSizeProp, RaycastingProcessor* raycaster)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputVolume("InputVolume", "Input Volume", "", DataNameProperty::READ)
        , p_camera("Camera", "Camera ID", "camera", DataNameProperty::READ)
        , p_outputImage("OutputImage", "Output Image", "vr.output", DataNameProperty::WRITE)
        , p_profileRaycaster("ProfileRaycaster", "Profile Raycaster's Execution Time", false)
        , _timerQueryRaycaster(0)
        , p_pgProps("PGGProps", "Proxy Geometry Generator")
        , p_eepProps("EEPProps", "Entry/Exit Points Generator")
        , p_raycasterProps("RaycasterProps", "Raycaster")
        , p_orientationOverlayProps("OrientationOverlayProps", "Orientation Overlay")
        , _pgGenerator()
        , _eepGenerator(viewportSizeProp)
        , _raycaster(raycaster)
        , _orientationOverlay(viewportSizeProp)
    {
        _raycaster->setViewportSizeProperty(viewportSizeProp);

        addProperty(p_inputVolume, AbstractProcessor::VALID);
        addProperty(p_outputImage, AbstractProcessor::VALID);
        addProperty(p_profileRaycaster, AbstractProcessor::VALID);

        p_pgProps.addPropertyCollection(_pgGenerator);
        _pgGenerator.p_sourceImageID.setVisible(false);
        _pgGenerator.p_geometryID.setVisible(false);
        addProperty(p_pgProps, AbstractProcessor::VALID);

        p_eepProps.addPropertyCollection(_eepGenerator);
        _eepGenerator.p_lqMode.setVisible(false);
        _eepGenerator.p_camera.setVisible(false);
        _eepGenerator.p_sourceImageID.setVisible(false);
        _eepGenerator.p_geometryID.setVisible(false);
        _eepGenerator.p_entryImageID.setVisible(false);
        _eepGenerator.p_exitImageID.setVisible(false);
        addProperty(p_eepProps, AbstractProcessor::VALID);

        const std::vector<std::string>& raycasters = ProcessorFactory::getRef().getRegisteredProcessors();
        for (int i = 0; i < raycasters.size(); i++) {
            p_raycastingProcSelector.addOption(GenericOption<std::string>(raycasters[i], raycasters[i]));
        }
        if (_raycaster != nullptr) {
            p_raycastingProcSelector.selectByOption(_raycaster->getName());
        }
        addProperty(p_raycastingProcSelector);

        p_raycasterProps.addPropertyCollection(*_raycaster);
        _raycaster->p_lqMode.setVisible(false);
        _raycaster->p_camera.setVisible(false);
        _raycaster->p_sourceImageID.setVisible(false);
        _raycaster->p_entryImageID.setVisible(false);
        _raycaster->p_exitImageID.setVisible(false);
        _raycaster->p_targetImageID.setVisible(false);
        addProperty(p_raycasterProps, AbstractProcessor::VALID);

        p_orientationOverlayProps.addPropertyCollection(_orientationOverlay);
        addProperty(p_orientationOverlayProps, VALID);

        // setup shared properties
        p_inputVolume.addSharedProperty(&_pgGenerator.p_sourceImageID);
        p_inputVolume.addSharedProperty(&_eepGenerator.p_sourceImageID);
        p_inputVolume.addSharedProperty(&_raycaster->p_sourceImageID);
        p_inputVolume.addSharedProperty(&_orientationOverlay.p_sourceImageId);

        p_camera.addSharedProperty(&_eepGenerator.p_camera);
        p_camera.addSharedProperty(&_raycaster->p_camera);
        p_camera.addSharedProperty(&_orientationOverlay.p_camera);

        p_outputImage.addSharedProperty(&_orientationOverlay.p_targetImageId);


        s_processorCanBeDeleted.connect(this, &VolumeRenderer::onProcessorCanBeDeleted);
    }

    VolumeRenderer::~VolumeRenderer() {
        delete _raycaster;
    }

    void VolumeRenderer::init() {
        VisualizationProcessor::init();
        _pgGenerator.init();
        _eepGenerator.init();
        _orientationOverlay.init();
        _raycaster->init();

        p_lqMode.addSharedProperty(&_raycaster->p_lqMode);

        _pgGenerator.s_invalidated.connect(this, &VolumeRenderer::onProcessorInvalidated);
        _eepGenerator.s_invalidated.connect(this, &VolumeRenderer::onProcessorInvalidated);
        _raycaster->s_invalidated.connect(this, &VolumeRenderer::onProcessorInvalidated);
        _orientationOverlay.s_invalidated.connect(this, &VolumeRenderer::onProcessorInvalidated);

        glGenQueries(1, &_timerQueryRaycaster);
    }

    void VolumeRenderer::deinit() {
        glDeleteQueries(1, &_timerQueryRaycaster);

        _pgGenerator.s_invalidated.disconnect(this);
        _eepGenerator.s_invalidated.disconnect(this);
        _raycaster->s_invalidated.disconnect(this);
        _orientationOverlay.s_invalidated.disconnect(this);

        _pgGenerator.deinit();
        _eepGenerator.deinit();
        _raycaster->deinit();
        _orientationOverlay.deinit();

        VisualizationProcessor::deinit();
    }

    void VolumeRenderer::updateResult(DataContainer& data) {
        if (getInvalidationLevel() & PG_INVALID) {
            _pgGenerator.process(data);
        }
        if (getInvalidationLevel() & EEP_INVALID) {
            _eepGenerator.process(data);
        }
        if (getInvalidationLevel() & RAYCASTER_INVALID) {
            if (p_profileRaycaster.getValue()) {
                glBeginQuery(GL_TIME_ELAPSED, _timerQueryRaycaster);
                _raycaster->process(data);
                glEndQuery(GL_TIME_ELAPSED);

                GLuint64 timer_result;
                glGetQueryObjectui64v(_timerQueryRaycaster, GL_QUERY_RESULT, &timer_result);
                LINFO("Raycaster Execution time: " << static_cast<double>(timer_result) / 1e06 << "ms.");
            }
            else {
                _raycaster->process(data);
            }            
        }

        _orientationOverlay.process(data);

        validate(INVALID_RESULT | PG_INVALID | EEP_INVALID | RAYCASTER_INVALID);
    }

    void VolumeRenderer::onProcessorInvalidated(AbstractProcessor* processor) {
        if (processor == &_pgGenerator) {
            invalidate(PG_INVALID | EEP_INVALID | RAYCASTER_INVALID);
        }
        else if (processor == &_eepGenerator) {
            invalidate(EEP_INVALID | RAYCASTER_INVALID);
        }
        else if (processor == _raycaster) {
            invalidate(RAYCASTER_INVALID);
        }

        invalidate(AbstractProcessor::INVALID_RESULT);
    }

    void VolumeRenderer::onPropertyChanged(const AbstractProperty* prop) {
        if (prop == &p_outputImage) {
            _pgGenerator.p_geometryID.setValue(p_outputImage.getValue() + ".geometry");
            _eepGenerator.p_geometryID.setValue(p_outputImage.getValue() + ".geometry");

            _eepGenerator.p_entryImageID.setValue(p_outputImage.getValue() + ".entrypoints");
            _raycaster->p_entryImageID.setValue(p_outputImage.getValue() + ".entrypoints");

            _eepGenerator.p_exitImageID.setValue(p_outputImage.getValue() + ".exitpoints");
            _raycaster->p_exitImageID.setValue(p_outputImage.getValue() + ".exitpoints");

            _raycaster->p_targetImageID.setValue(p_outputImage.getValue() + ".raycasted");
            _orientationOverlay.p_passThroughImageId.setValue(p_outputImage.getValue() + ".raycasted");
        }
        if (prop == &p_raycastingProcSelector) {
            RaycastingProcessor *currentRaycaster = _raycaster;
            // Change to previous raycaster if "Select Processor" is selected
            if (p_raycastingProcSelector.getOptionId() == p_raycastingProcSelector.getOptions()[0]._id) {
                p_raycastingProcSelector.selectById(currentRaycaster->getName());
                return;
            }
            if (p_raycastingProcSelector.getOptionId() == currentRaycaster->getName()) {
                return;
            }

            p_lqMode.removeSharedProperty(&currentRaycaster->p_lqMode);
            p_inputVolume.removeSharedProperty(&currentRaycaster->p_sourceImageID);
            p_camera.removeSharedProperty(&currentRaycaster->p_camera);
            p_outputImage.removeSharedProperty(&currentRaycaster->p_targetImageID);
            p_raycasterProps.clearProperties();
            currentRaycaster->s_invalidated.disconnect(this);
            
            _raycaster = dynamic_cast<RaycastingProcessor*>(ProcessorFactory::getRef().createProcessor(p_raycastingProcSelector.getOptionId(), _viewportSizeProperty));
            cgtAssert(_raycaster != 0, "Raycaster must not be 0.");

            p_raycasterProps.addPropertyCollection(*_raycaster);
            //_raycaster->p_lqMode.setVisible(false);
            //_raycaster->p_camera.setVisible(false);
            //_raycaster->p_sourceImageID.setVisible(false);
            //_raycaster->p_entryImageID.setVisible(false);
            //_raycaster->p_exitImageID.setVisible(false);
            //_raycaster->p_targetImageID.setVisible(false);

            p_lqMode.addSharedProperty(&_raycaster->p_lqMode);
            p_inputVolume.addSharedProperty(&_raycaster->p_sourceImageID);
            p_camera.addSharedProperty(&_raycaster->p_camera);
            p_outputImage.addSharedProperty(&_raycaster->p_targetImageID);
            _raycaster->s_invalidated.connect(this, &VolumeRenderer::onProcessorInvalidated);
            
            cgt::OpenGLJobProcessor::ScopedSynchronousGlJobExecution jobGuard;
            _raycaster->init();
            _raycaster->p_sourceImageID.setValue(currentRaycaster->p_sourceImageID.getValue());
            _raycaster->p_entryImageID.setValue(currentRaycaster->p_entryImageID.getValue());
            _raycaster->p_exitImageID.setValue(currentRaycaster->p_exitImageID.getValue());
            _raycaster->p_targetImageID.setValue(currentRaycaster->p_targetImageID.getValue());
            _raycaster->p_camera.setValue(currentRaycaster->p_camera.getValue());
            _raycaster->p_transferFunction.replaceTF(currentRaycaster->p_transferFunction.getTF()->clone());
            _raycaster->p_jitterStepSizeMultiplier.setValue(currentRaycaster->p_jitterStepSizeMultiplier.getValue());
            _raycaster->p_samplingRate.setValue(currentRaycaster->p_samplingRate.getValue());

            currentRaycaster->deinit();
            invalidate(RAYCASTER_INVALID);

            // queue the deletion of currentRaycaster as signal, to ensure that the deletion does
            // not happen before all previously emitted signals have been handled.
            s_processorCanBeDeleted.queueSignal(currentRaycaster);
        }

        VisualizationProcessor::onPropertyChanged(prop);
    }

    void VolumeRenderer::setViewportSizeProperty(IVec2Property* viewportSizeProp) {
        _eepGenerator.setViewportSizeProperty(viewportSizeProp);
        _raycaster->setViewportSizeProperty(viewportSizeProp);
        _orientationOverlay.setViewportSizeProperty(viewportSizeProp);

        VisualizationProcessor::setViewportSizeProperty(viewportSizeProp);
    }

    void VolumeRenderer::updateProperties(DataContainer& dataContainer) {
        // nothing to do here
    }

    RaycastingProcessor* VolumeRenderer::getRaycastingProcessor() {
        return _raycaster;
    }

    void VolumeRenderer::onProcessorCanBeDeleted(AbstractProcessor* processor) {
        delete processor;
    }

}

