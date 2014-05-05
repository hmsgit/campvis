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
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"

#include "core/classification/simpletransferfunction.h"

namespace campvis {
    const std::string VolumeRenderer::loggerCat_ = "CAMPVis.modules.vis.VolumeRenderer";

    VolumeRenderer::VolumeRenderer(IVec2Property* viewportSizeProp, RaycastingProcessor* raycaster)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputVolume("InputVolume", "Input Volume", "", DataNameProperty::READ)
        , p_camera("Camera", "Camera", tgt::Camera())
        , p_outputImage("OutputImage", "Output Image", "vr.output", DataNameProperty::WRITE)
        , p_profileRaycaster("ProfileRaycaster", "Profile Raycaster's Execution Time", false)
        , _timerQueryRaycaster(0)
        , p_pgProps("PGGProps", "Proxy Geometry Generator")
        , p_eepProps("EEPProps", "Entry/Exit Points Generator")
        , p_raycasterProps("RaycasterProps", "Raycaster")
        , _pgGenerator()
        , _eepGenerator(viewportSizeProp)
        , _raycaster(raycaster)
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

        p_raycasterProps.addPropertyCollection(*_raycaster);
        _raycaster->p_lqMode.setVisible(false);
        _raycaster->p_camera.setVisible(false);
        _raycaster->p_sourceImageID.setVisible(false);
        _raycaster->p_entryImageID.setVisible(false);
        _raycaster->p_exitImageID.setVisible(false);
        _raycaster->p_targetImageID.setVisible(false);
        addProperty(p_raycasterProps, AbstractProcessor::VALID);

        // setup shared properties
        p_inputVolume.addSharedProperty(&_pgGenerator.p_sourceImageID);
        p_inputVolume.addSharedProperty(&_eepGenerator.p_sourceImageID);
        p_inputVolume.addSharedProperty(&_raycaster->p_sourceImageID);

        p_camera.addSharedProperty(&_eepGenerator.p_camera);
        p_camera.addSharedProperty(&_raycaster->p_camera);

        p_outputImage.addSharedProperty(&_raycaster->p_targetImageID);

        p_inputVolume.s_changed.connect(this, &VolumeRenderer::onPropertyChanged);
    }

    VolumeRenderer::~VolumeRenderer() {

    }

    void VolumeRenderer::init() {
        VisualizationProcessor::init();
        _pgGenerator.init();
        _eepGenerator.init();
        _raycaster->init();

        p_lqMode.addSharedProperty(&_raycaster->p_lqMode);

        _pgGenerator.s_invalidated.connect(this, &VolumeRenderer::onProcessorInvalidated);
        _eepGenerator.s_invalidated.connect(this, &VolumeRenderer::onProcessorInvalidated);
        _raycaster->s_invalidated.connect(this, &VolumeRenderer::onProcessorInvalidated);

        glGenQueries(1, &_timerQueryRaycaster);
    }

    void VolumeRenderer::deinit() {
        glDeleteQueries(1, &_timerQueryRaycaster);

        _pgGenerator.s_invalidated.disconnect(this);
        _eepGenerator.s_invalidated.disconnect(this);
        _raycaster->s_invalidated.disconnect(this);

        _pgGenerator.deinit();
        _eepGenerator.deinit();
        _raycaster->deinit();

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
        }
        VisualizationProcessor::onPropertyChanged(prop);
    }

    void VolumeRenderer::setViewportSizeProperty(IVec2Property* viewportSizeProp) {
        _eepGenerator.setViewportSizeProperty(viewportSizeProp);
        _raycaster->setViewportSizeProperty(viewportSizeProp);

        VisualizationProcessor::setViewportSizeProperty(viewportSizeProp);
    }

    void VolumeRenderer::updateProperties(DataContainer& dataContainer) {
        // nothing to do here
    }

    RaycastingProcessor* VolumeRenderer::getRaycastingProcessor() {
        return _raycaster;
    }
}

