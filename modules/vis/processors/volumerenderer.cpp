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

#include "volumerenderer.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"

#include "core/classification/simpletransferfunction.h"

namespace campvis {
    const std::string VolumeRenderer::loggerCat_ = "CAMPVis.modules.vis.VolumeRenderer";

    VolumeRenderer::VolumeRenderer(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputVolume("InputVolume", "Input Volume", "", DataNameProperty::READ, AbstractProcessor::VALID)
        , p_camera("Camera", "Camera", tgt::Camera(), AbstractProcessor::VALID)
        , p_outputImage("OutputImage", "Output Image", "vr.output", DataNameProperty::WRITE, AbstractProcessor::VALID)
        , p_pgProps("PGGProps", "Proxy Geometry Generator", AbstractProcessor::VALID)
        , p_eepProps("EEPProps", "Entry/Exit Points Generator", AbstractProcessor::VALID)
        , p_raycasterProps("RaycasterProps", "Raycaster", AbstractProcessor::VALID)
        , _pgGenerator()
        , _eepGenerator(viewportSizeProp)
        , _raycaster(viewportSizeProp)
    {
        addProperty(&p_inputVolume);
        addProperty(&_raycaster.p_transferFunction);
        addProperty(&p_outputImage);

        p_pgProps.addPropertyCollection(_pgGenerator);
        _pgGenerator.p_sourceImageID.setVisible(false);
        _pgGenerator.p_geometryID.setVisible(false);
        addProperty(&p_pgProps);

        p_eepProps.addPropertyCollection(_eepGenerator);
        _eepGenerator.p_lqMode.setVisible(false);
        _eepGenerator.p_camera.setVisible(false);
        _eepGenerator.p_sourceImageID.setVisible(false);
        _eepGenerator.p_geometryID.setVisible(false);
        _eepGenerator.p_entryImageID.setVisible(false);
        _eepGenerator.p_exitImageID.setVisible(false);
        addProperty(&p_eepProps);

        p_raycasterProps.addPropertyCollection(_raycaster);
        _raycaster.p_lqMode.setVisible(false);
        _raycaster.p_camera.setVisible(false);
        _raycaster.p_sourceImageID.setVisible(false);
        _raycaster.p_entryImageID.setVisible(false);
        _raycaster.p_exitImageID.setVisible(false);
        _raycaster.p_targetImageID.setVisible(false);
        addProperty(&p_raycasterProps);

        // setup shared properties
        p_inputVolume.addSharedProperty(&_pgGenerator.p_sourceImageID);
        p_inputVolume.addSharedProperty(&_eepGenerator.p_sourceImageID);
        p_inputVolume.addSharedProperty(&_raycaster.p_sourceImageID);

        p_camera.addSharedProperty(&_eepGenerator.p_camera);
        p_camera.addSharedProperty(&_raycaster.p_camera);

        p_outputImage.addSharedProperty(&_raycaster.p_targetImageID);

        p_inputVolume.s_changed.connect(this, &VolumeRenderer::onPropertyChanged);
    }

    VolumeRenderer::~VolumeRenderer() {

    }

    void VolumeRenderer::init() {
        VisualizationProcessor::init();
        _pgGenerator.init();
        _eepGenerator.init();
        _raycaster.init();

        p_lqMode.addSharedProperty(&_raycaster.p_lqMode);

        _pgGenerator.s_invalidated.connect(this, &VolumeRenderer::onProcessorInvalidated);
        _eepGenerator.s_invalidated.connect(this, &VolumeRenderer::onProcessorInvalidated);
        _raycaster.s_invalidated.connect(this, &VolumeRenderer::onProcessorInvalidated);
    }

    void VolumeRenderer::deinit() {
        _pgGenerator.s_invalidated.disconnect(this);
        _eepGenerator.s_invalidated.disconnect(this);
        _raycaster.s_invalidated.disconnect(this);

        _pgGenerator.deinit();
        _eepGenerator.deinit();
        _raycaster.deinit();

        VisualizationProcessor::deinit();
    }

    void VolumeRenderer::process(DataContainer& data) {
        if (getInvalidationLevel() & PG_INVALID) {
            _pgGenerator.process(data);
            _eepGenerator.process(data);
            _raycaster.process(data);
        }
        else if (getInvalidationLevel() & EEP_INVALID) {
            _eepGenerator.process(data);
            _raycaster.process(data);
        }
        else if (getInvalidationLevel() & RAYCASTER_INVALID) {
            _raycaster.process(data);
        }

        validate(INVALID_RESULT | PG_INVALID | EEP_INVALID | RAYCASTER_INVALID);
    }

    void VolumeRenderer::onProcessorInvalidated(AbstractProcessor* processor) {
        if (processor == &_pgGenerator) {
            invalidate(PG_INVALID);
        }
        else if (processor == &_eepGenerator) {
            invalidate(EEP_INVALID);
        }
        else if (processor == &_raycaster) {
            invalidate(RAYCASTER_INVALID);
        }

        invalidate(AbstractProcessor::INVALID_RESULT);
    }

    void VolumeRenderer::onPropertyChanged(const AbstractProperty* prop) {
        if (prop == &p_outputImage) {
            _pgGenerator.p_geometryID.setValue(p_outputImage.getValue() + ".geometry");
            _eepGenerator.p_geometryID.setValue(p_outputImage.getValue() + ".geometry");

            _eepGenerator.p_entryImageID.setValue(p_outputImage.getValue() + ".entrypoints");
            _raycaster.p_entryImageID.setValue(p_outputImage.getValue() + ".entrypoints");

            _eepGenerator.p_exitImageID.setValue(p_outputImage.getValue() + ".exitpoints");
            _raycaster.p_exitImageID.setValue(p_outputImage.getValue() + ".exitpoints");
        }
        else if (prop == &p_inputVolume) {
            invalidate(AbstractProcessor::INVALID_RESULT | PG_INVALID);
        }
        VisualizationProcessor::onPropertyChanged(prop);
    }

    void VolumeRenderer::setViewportSizeProperty(IVec2Property* viewportSizeProp) {
        _eepGenerator.setViewportSizeProperty(viewportSizeProp);
        _raycaster.setViewportSizeProperty(viewportSizeProp);

        VisualizationProcessor::setViewportSizeProperty(viewportSizeProp);
    }

}

