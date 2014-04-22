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

#include "strainfiberrenderer.h"

#include "tgt/glmath.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"


#include "modules/columbia/datastructures/fiberdata.h"
#include "core/datastructures/lightsourcedata.h"
#include "core/datastructures/renderdata.h"
#include "core/datastructures/meshgeometry.h"

namespace campvis {

    static const GenericOption<StrainFiberRenderer::RenderMode> renderModeOptions[2] = {
        GenericOption<StrainFiberRenderer::RenderMode>("Stripes", "Stripes", StrainFiberRenderer::STRIPES), 
        GenericOption<StrainFiberRenderer::RenderMode>("Tubes", "Tubes", StrainFiberRenderer::TUBES)
    };

    const std::string StrainFiberRenderer::loggerCat_ = "CAMPVis.modules.vis.StrainFiberRenderer";

    StrainFiberRenderer::StrainFiberRenderer(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_strainId("StrainDataId", "Input Strain Data ID", "gr.strain", DataNameProperty::READ)
        , p_renderTargetID("p_renderTargetID", "Output Image", "gr.output", DataNameProperty::WRITE)
        , p_camera("Camera", "Camera ID")
        , p_renderMode("RenderMode", "Render Mode", renderModeOptions, 2)
        , p_lineWidth("LineWidth", "Line width", 3.f, .5f, 10.f, 0.1f)
        , p_color("color", "Rendering Color", tgt::vec4(1.f), tgt::vec4(0.f), tgt::vec4(1.f))
        , p_enableShading("EnableShading", "Enable Shading", true)
        , p_lightId("LightId", "Input Light Source", "lightsource", DataNameProperty::READ)
        , _shader(0)
    {
        addProperty(p_strainId);
        addProperty(p_renderTargetID);
        addProperty(p_camera);
        addProperty(p_color);
        addProperty(p_renderMode, INVALID_RESULT | INVALID_SHADER);
        addProperty(p_lineWidth);

        addProperty(p_enableShading, INVALID_RESULT | INVALID_PROPERTIES | INVALID_SHADER);
        addProperty(p_lightId);
    }

    StrainFiberRenderer::~StrainFiberRenderer() {

    }

    void StrainFiberRenderer::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.load("modules/columbia/glsl/strainfiberrenderer.vert", "modules/columbia/glsl/strainfiberrenderer.geom", "modules/columbia/glsl/strainfiberrenderer.frag", generateGlslHeader());
        if (_shader != 0) {
            _shader->setAttributeLocation(0, "in_Position");
            _shader->setAttributeLocation(1, "in_TexCoord");
        }
        invalidate(AbstractProcessor::INVALID_SHADER);
    }

    void StrainFiberRenderer::deinit() {
        ShdrMgr.dispose(_shader);
        _shader = 0;
        VisualizationProcessor::deinit();
    }

    void StrainFiberRenderer::updateResult(DataContainer& data) {
        ScopedTypedData<FiberData> strainData(data, p_strainId.getValue());
        ScopedTypedData<LightSourceData> light(data, p_lightId.getValue());

        if (strainData != 0 && _shader != 0) {
            if (p_enableShading.getValue() == false || light != nullptr) {
                const tgt::Camera& camera = p_camera.getValue();

                // set modelview and projection matrices
                FramebufferActivationGuard fag(this);
                createAndAttachColorTexture();
                createAndAttachDepthTexture();

                _shader->activate();
                _shader->setIgnoreUniformLocationError(true);
                if (p_enableShading.getValue() && light != nullptr) {
                    light->bind(_shader, "_lightSource");
                }
                _shader->setUniform("_projectionMatrix", camera.getProjectionMatrix());
                _shader->setUniform("_viewMatrix", camera.getViewMatrix());
                _shader->setUniform("_cameraPosition", camera.getPosition());
                _shader->setUniform("_fiberWidth", p_lineWidth.getValue()/4.f);
                _shader->setIgnoreUniformLocationError(false); 

                // create entry points texture
                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LESS);
                glClearDepth(1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                glLineWidth(p_lineWidth.getValue());
                strainData->render();
                glLineWidth(1.f);

                _shader->deactivate();
                glDisable(GL_DEPTH_TEST);
                LGL_ERROR;

                data.addData(p_renderTargetID.getValue(), new RenderData(_fbo));
            }
            else {
                LDEBUG("Could not load light source from DataContainer.");
            }
        }
        else {
            LERROR("No suitable input geometry found.");
        }

        validate(INVALID_RESULT);
    }

    std::string StrainFiberRenderer::generateGlslHeader() const {
        std::string toReturn;

        if (p_enableShading.getValue())
            toReturn += "#define ENABLE_SHADING\n";

        switch (p_renderMode.getOptionValue()) {
            case STRIPES:
                toReturn += "#define DO_STRIPES\n";
                break;
            case TUBES:
                toReturn += "#define DO_TUBES\n";
                break;

        }

        return toReturn;
    }

    void StrainFiberRenderer::updateShader() {
        _shader->setHeaders(generateGlslHeader());
        _shader->rebuild();
        validate(INVALID_SHADER);
    }

    void StrainFiberRenderer::updateProperties(DataContainer& dataContainer) {
        p_lightId.setVisible(p_enableShading.getValue());
        validate(AbstractProcessor::INVALID_PROPERTIES);
    }

}
