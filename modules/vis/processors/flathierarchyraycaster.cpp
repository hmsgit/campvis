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

#include "flathierarchyraycaster.h"

#include "core/tools/flathierarchymapper.h"
#include "core/tools/quadrenderer.h"

#include "core/datastructures/cameradata.h"
#include "core/datastructures/lightsourcedata.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorgradient.h"

#include <tbb/tbb.h>

namespace campvis {
    const std::string FlatHierarchyRaycaster::loggerCat_ = "CAMPVis.modules.vis.FlatHierarchyRaycaster";

    FlatHierarchyRaycaster::FlatHierarchyRaycaster(IVec2Property* viewportSizeProp)
        : RaycastingProcessor(viewportSizeProp, "modules/vis/glsl/flathierarchyraycaster.frag", true)
        , p_enableShading("EnableShading", "Enable Shading", true)
        , p_lightId("LightId", "Input Light Source", "lightsource", DataNameProperty::READ)
        , p_numBlocks("NumBlocks", "Number of Blocks in LOD Selection", cgt::ivec3(8), cgt::ivec3(1), cgt::ivec3(32))
        , _fhm(nullptr)
    {
        addDecorator(new ProcessorDecoratorGradient());

        addProperty(p_enableShading, INVALID_RESULT | INVALID_SHADER);
        addProperty(p_lightId);
        addProperty(p_numBlocks, INVALID_RESULT | INVALID_LOD);

        setPropertyInvalidationLevel(p_sourceImageID, INVALID_RESULT | INVALID_HIERARCHY | INVALID_LOD);
        setPropertyInvalidationLevel(p_transferFunction, INVALID_RESULT | INVALID_LOD);
        decoratePropertyCollection(this);
    }

    FlatHierarchyRaycaster::~FlatHierarchyRaycaster() {
    }

    void FlatHierarchyRaycaster::init() {
        RaycastingProcessor::init();

        invalidate(INVALID_HIERARCHY | INVALID_LOD);
    }

    void FlatHierarchyRaycaster::deinit() {
        delete _fhm;
        RaycastingProcessor::deinit();
    }

    void FlatHierarchyRaycaster::updateResult(DataContainer& data) {
        ScopedTypedData<ImageData> img(data, p_sourceImageID.getValue());
        ScopedTypedData<RenderData> entryPoints(data, p_entryImageID.getValue());
        ScopedTypedData<RenderData> exitPoints(data, p_exitImageID.getValue());
        ScopedTypedData<CameraData> camera(data, p_camera.getValue());
        ScopedTypedData<LightSourceData> light(data, p_lightId.getValue());

        if ( !(img && entryPoints && exitPoints && camera && light) ) {
            LERROR("Could not load all the necessary data from the DataContainer.");
            return;
        }

        if (getInvalidationLevel() & INVALID_HIERARCHY) {
            delete _fhm;
            _fhm = AbstractFlatHierarchyMapper::create(img);
            validate(INVALID_HIERARCHY);
        }

        if (_fhm == nullptr) {
            LDEBUG("The FlatHierarchyMapper does not exists, something went wrong!");
            return;
        }

        if (getInvalidationLevel() & INVALID_LOD) {
            _fhm->selectLod(p_transferFunction.getTF(), cgt::svec3(p_numBlocks.getValue()));
            data.addDataHandle("FHM.LOD", _fhm->_flatHierarchyDH);
            data.addDataHandle("FHM.index", _fhm->_indexDH);
            validate(INVALID_LOD);
        }


        if (p_enableShading.getValue() == false || light != nullptr) {
            _shader->activate();
            _shader->setIgnoreUniformLocationError(true);

            decorateRenderProlog(data, _shader);
            _shader->setUniform("_viewportSizeRCP", 1.f / cgt::vec2(getEffectiveViewportSize()));
            _shader->setUniform("_jitterStepSizeMultiplier", p_jitterStepSizeMultiplier.getValue());

            // compute sampling step size relative to volume size
            float samplingStepSize = 1.f / (p_samplingRate.getValue() * cgt::max(img->getSize()));
            _shader->setUniform("_samplingStepSize", samplingStepSize);

            // compute and set camera parameters
            const cgt::Camera& cam = camera->getCamera();
            float n = cam.getNearDist();
            float f = cam.getFarDist();
            _shader->setUniform("_cameraPosition", cam.getPosition());
            _shader->setUniform("const_to_z_e_1", 0.5f + 0.5f*((f+n)/(f-n)));
            _shader->setUniform("const_to_z_e_2", ((f-n)/(f*n)));
            _shader->setUniform("const_to_z_w_1", ((f*n)/(f-n)));
            _shader->setUniform("const_to_z_w_2", 0.5f*((f+n)/(f-n))+0.5f);

            // bind input textures
            cgt::TextureUnit indexUnit, lodUnit, tfUnit;
            indexUnit.activate();
            _fhm->getIndexTexture()->bind();
            _shader->setUniform("_indexTexture", indexUnit.getUnitNumber());

            _shader->setUniform("_indexTextureSize", cgt::vec3(_fhm->getIndexTexture()->getDimensions()));
            _shader->setUniform("_lodTextureSize", cgt::vec3(_fhm->getFlatHierarchyTexture()->getDimensions()));
            _shader->setUniform("_nonNpotVolumeCompensationMultiplier", (cgt::vec3(_fhm->getIndexTexture()->getDimensions()) * 16.f) / cgt::vec3(img->getSize()));

            _shader->setUniform("_volumeTextureParams._size", cgt::vec3(img->getSize()));
            _shader->setUniform("_volumeTextureParams._sizeRCP", cgt::vec3(1.f) / cgt::vec3(img->getSize()));
            _shader->setUniform("_volumeTextureParams._numChannels", static_cast<int>(img->getNumChannels()));
            _shader->setUniform("_volumeTextureParams._voxelSize", img->getMappingInformation().getVoxelSize());
            _shader->setUniform("_volumeTextureParams._voxelSizeRCP", cgt::vec3(1.f) / img->getMappingInformation().getVoxelSize());
            _shader->setUniform("_volumeTextureParams._textureToWorldMatrix", img->getMappingInformation().getTextureToWorldMatrix());
            _shader->setUniform("_volumeTextureParams._worldToTextureMatrix", img->getMappingInformation().getWorldToTextureMatrix());

            lodUnit.activate();
            _fhm->getFlatHierarchyTexture()->bind();
            _shader->setUniform("_lodTexture", lodUnit.getUnitNumber());

            p_transferFunction.getTF()->bind(_shader, tfUnit);

            cgt::TextureUnit entryUnit, exitUnit, entryUnitDepth, exitUnitDepth;
            entryPoints->bind(_shader, entryUnit, entryUnitDepth, "_entryPoints", "_entryPointsDepth", "_entryParams");
            exitPoints->bind(_shader, exitUnit, exitUnitDepth, "_exitPoints", "_exitPointsDepth", "_exitParams");

            if (p_enableShading.getValue() && light != nullptr) {
                light->bind(_shader, "_lightSource");
            }

            _shader->setIgnoreUniformLocationError(false);

            FramebufferActivationGuard fag(this);
            createAndAttachTexture(GL_RGBA8);
            createAndAttachTexture(GL_RGBA32F);
            createAndAttachTexture(GL_RGBA32F);
            createAndAttachDepthTexture();

            static const GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 , GL_COLOR_ATTACHMENT2 };
            glDrawBuffers(3, buffers);

            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            QuadRdr.renderQuad();

            // restore state
            glDrawBuffers(1, buffers);
            glDisable(GL_DEPTH_TEST);
            LGL_ERROR;

            data.addData(p_targetImageID.getValue(), new RenderData(_fbo));

            decorateRenderEpilog(_shader);
            _shader->deactivate();
            cgt::TextureUnit::setZeroUnit();
            LGL_ERROR;
        }
        else {
            LDEBUG("Could not load light source from DataContainer.");
        }
    }

    void FlatHierarchyRaycaster::processImpl(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image) {
        cgtAssert(false, "Should not reach this, since this processor has it's own updateResult() overload!");
    }

    std::string FlatHierarchyRaycaster::generateHeader() const {
        std::string toReturn = RaycastingProcessor::generateHeader();
        if (p_enableShading.getValue())
            toReturn += "#define ENABLE_SHADING\n";
        return toReturn;
    }


}
