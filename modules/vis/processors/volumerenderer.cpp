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
        , p_raycasterProps("RaycasterProps", "Raycaster", AbstractProcessor::VALID)
        , _pgGenerator()
        , _eepGenerator(viewportSizeProp)
        , _raycaster(viewportSizeProp)
    {
        addProperty(&p_inputVolume);
        addProperty(&p_camera);
        addProperty(&_raycaster.p_transferFunction);
        addProperty(&_raycaster.p_samplingRate);
        addProperty(&p_outputImage);

        addProperty(_raycaster.getProperty("CentralDifferences"));
        addProperty(_eepGenerator.getProperty("GeometryImageId"));

        p_raycasterProps.addPropertyCollection(_raycaster);
        addProperty(&p_raycasterProps);

        // setup shared properties
        p_inputVolume.addSharedProperty(&_pgGenerator.p_sourceImageID);
        p_inputVolume.addSharedProperty(&_eepGenerator.p_sourceImageID);
        p_inputVolume.addSharedProperty(&_raycaster.p_sourceImageID);

        p_camera.addSharedProperty(&_eepGenerator.p_camera);
        p_camera.addSharedProperty(&_raycaster.p_camera);

        p_outputImage.addSharedProperty(&_raycaster.p_targetImageID);

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

        removeProperty(&_raycaster.p_transferFunction);
        removeProperty(&_raycaster.p_samplingRate);
        removeProperty(_raycaster.getProperty("CentralDifferences"));

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
        VisualizationProcessor::onPropertyChanged(prop);
    }

    void VolumeRenderer::setViewportSizeProperty(IVec2Property* viewportSizeProp) {
        _eepGenerator.setViewportSizeProperty(viewportSizeProp);
        _raycaster.setViewportSizeProperty(viewportSizeProp);

        VisualizationProcessor::setViewportSizeProperty(viewportSizeProp);
    }

}

