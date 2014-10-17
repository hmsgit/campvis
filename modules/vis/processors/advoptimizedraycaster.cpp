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

#include "advoptimizedraycaster.h"

#include "core/tools/quadrenderer.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorgradient.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/lightsourcedata.h"
#include "core/datastructures/geometrydatafactory.h"

namespace campvis {
    const std::string AdvOptimizedRaycaster::loggerCat_ = "CAMPVis.modules.vis.AdvOptimizedRaycaster";

    AdvOptimizedRaycaster::AdvOptimizedRaycaster(IVec2Property* viewportSizeProp)
        : RaycastingProcessor(viewportSizeProp, "modules/vis/glsl/advoptimizedraycaster.frag", true, "400")
        , p_enableShading("EnableShading", "Enable Shading", true)
        , p_lightId("LightId", "Input Light Source", "lightsource", DataNameProperty::READ)
        , _vhm(nullptr)
    {
        addDecorator(new ProcessorDecoratorGradient());

        addProperty(p_enableShading, INVALID_RESULT | INVALID_PROPERTIES | INVALID_SHADER);
        addProperty(p_lightId);

        setPropertyInvalidationLevel(p_transferFunction, INVALID_BBV | INVALID_RESULT);
        setPropertyInvalidationLevel(p_sourceImageID, INVALID_BBV | INVALID_PROPERTIES | INVALID_RESULT);

        decoratePropertyCollection(this);
    }

    AdvOptimizedRaycaster::~AdvOptimizedRaycaster() {
    }

    void AdvOptimizedRaycaster::init() {
        RaycastingProcessor::init();

        _vhm = new VoxelHierarchyMapper();
        invalidate(INVALID_BBV);
    }

    void AdvOptimizedRaycaster::deinit() {
        delete _vhm;
        RaycastingProcessor::deinit();
    }

    void AdvOptimizedRaycaster::processImpl(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image) {
        _shader->deactivate();

        if (getInvalidationLevel() & INVALID_BBV){
            _vhm->createHierarchy(image, p_transferFunction.getTF());
            validate(INVALID_BBV);
        }
        
        if (_vhm->getTexture() == nullptr) {
            LERROR("Could not retreive voxel hierarchy lookup structure.");
            return;
        }

        ScopedTypedData<LightSourceData> light(data, p_lightId.getValue());

        if (p_enableShading.getValue() == false || light != nullptr) {
            _shader->activate();

            tgt::TextureUnit bbvUnit;
            bbvUnit.activate();
            _vhm->getTexture()->bind();
            {
                tgt::Shader::IgnoreUniformLocationErrorGuard guard(_shader);
                _shader->setUniform("_vvTexture", bbvUnit.getUnitNumber());
                _shader->setUniform("_vvVoxelSize", static_cast<int>(_vhm->getBrickSize()));
                _shader->setUniform("_vvVoxelDepth", static_cast<int>(_vhm->getBrickDepth()));
                _shader->setUniform("_vvMaxMipMapLevel", static_cast<int>(_vhm->getMaxMipmapLevel()));
            }

            if (p_enableShading.getValue() && light != nullptr) {
                light->bind(_shader, "_lightSource");
            }

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

            glDisable(GL_DEPTH_TEST);
            LGL_ERROR;

            glDrawBuffers(1, buffers);

            data.addData(p_targetImageID.getValue(), new RenderData(_fbo)); 
        }
    }

    std::string AdvOptimizedRaycaster::generateHeader() const {
        std::string toReturn = RaycastingProcessor::generateHeader();
        if (p_enableShading.getValue())
            toReturn += "#define ENABLE_SHADING\n";
        return toReturn;
    }

    void AdvOptimizedRaycaster::updateProperties(DataContainer& dataContainer) {
        RaycastingProcessor::updateProperties(dataContainer);
        validate(AbstractProcessor::INVALID_PROPERTIES);
    }

}
