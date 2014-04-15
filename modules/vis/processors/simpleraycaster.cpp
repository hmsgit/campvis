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

#include "simpleraycaster.h"

#include "core/tools/quadrenderer.h"
#include "core/datastructures/lightsourcedata.h"
#include "core/datastructures/renderdata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/pipeline/processordecoratorgradient.h"

#include <tbb/tbb.h>

namespace campvis {
    const std::string SimpleRaycaster::loggerCat_ = "CAMPVis.modules.vis.SimpleRaycaster";

    SimpleRaycaster::SimpleRaycaster(IVec2Property* viewportSizeProp)
        : RaycastingProcessor(viewportSizeProp, "modules/vis/glsl/simpleraycaster.frag", true)
        , p_enableShading("EnableShading", "Enable Shading", true)
        , p_lightId("LightId", "Input Light Source", "lightsource", DataNameProperty::READ)
        , p_enableShadowing("EnableShadowing", "Enable Hard Shadows (Expensive!)", false)
        , p_shadowIntensity("ShadowIntensity", "Shadow Intensity", .5f, .0f, 1.f)
    {
        addDecorator(new ProcessorDecoratorGradient());

        addProperty(p_enableShading, INVALID_RESULT | INVALID_PROPERTIES | INVALID_SHADER);
        addProperty(p_lightId);
        addProperty(p_enableShadowing, INVALID_RESULT | INVALID_PROPERTIES | INVALID_SHADER);
        addProperty(p_shadowIntensity);
        p_shadowIntensity.setVisible(false);

        decoratePropertyCollection(this);
    }

    SimpleRaycaster::~SimpleRaycaster() {
    }

    void SimpleRaycaster::init() {
        RaycastingProcessor::init();
    }

    void SimpleRaycaster::deinit() {
        RaycastingProcessor::deinit();
    }

    void SimpleRaycaster::processImpl(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image) {
        ScopedTypedData<LightSourceData> light(data, p_lightId.getValue());

        if (p_enableShading.getValue() == false || light != nullptr) {
            FramebufferActivationGuard fag(this);
            createAndAttachTexture(GL_RGBA8);
            createAndAttachTexture(GL_RGBA32F);
            createAndAttachTexture(GL_RGBA32F);
            createAndAttachDepthTexture();

            static const GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 , GL_COLOR_ATTACHMENT2 };
            glDrawBuffers(3, buffers);

            if (p_enableShading.getValue() && light != nullptr) {
                light->bind(_shader, "_lightSource");
            }
            if (p_enableShadowing.getValue()) {
                _shader->setUniform("_shadowIntensity", p_shadowIntensity.getValue());
            }

            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            QuadRdr.renderQuad();

            // restore state
            glDrawBuffers(1, buffers);
            glDisable(GL_DEPTH_TEST);
            LGL_ERROR;

            data.addData(p_targetImageID.getValue(), new RenderData(_fbo));


        
        }
        else {
            LDEBUG("Could not load light source from DataContainer.");
        }
    }

    std::string SimpleRaycaster::generateHeader() const {
        std::string toReturn = RaycastingProcessor::generateHeader();
        if (p_enableShading.getValue())
            toReturn += "#define ENABLE_SHADING\n";
        if (p_enableShadowing.getValue())
            toReturn += "#define ENABLE_SHADOWING\n";
        return toReturn;
    }

    void SimpleRaycaster::updateProperties(DataContainer& dataContainer) {
        RaycastingProcessor::updateProperties(dataContainer);
        p_lightId.setVisible(p_enableShading.getValue());
        p_shadowIntensity.setVisible(p_enableShadowing.getValue());
        validate(AbstractProcessor::INVALID_PROPERTIES);
    }

}
