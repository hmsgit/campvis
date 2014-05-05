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

#include "geometrystrainrenderer.h"

#include "tgt/glmath.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/lightsourcedata.h"
#include "core/datastructures/renderdata.h"
#include "core/datastructures/meshgeometry.h"

namespace campvis {
    const std::string GeometryStrainRenderer::loggerCat_ = "CAMPVis.modules.vis.GeometryStrainRenderer";

    GeometryStrainRenderer::GeometryStrainRenderer(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_geometryID("geometryID", "Input Geometry ID", "gr.geometry", DataNameProperty::READ)
        , p_strainId("StrainDataId", "Input Strain Data ID", "gr.strain", DataNameProperty::READ)
        , p_renderTargetID("p_renderTargetID", "Output Image", "gr.output", DataNameProperty::WRITE)
        , p_camera("camera", "Camera")
        , p_enableShading("EnableShading", "Enable Shading", true)
        , p_lightId("LightId", "Input Light Source", "lightsource", DataNameProperty::READ)
        , p_color("color", "Rendering Color", tgt::vec4(1.f), tgt::vec4(0.f), tgt::vec4(1.f))
        , _shader(0)
    {

        addProperty(p_geometryID);
        addProperty(p_renderTargetID);
        addProperty(p_camera);

        addProperty(p_enableShading, INVALID_RESULT | INVALID_PROPERTIES | INVALID_SHADER);
        addProperty(p_lightId);
        addProperty(p_color);
    }

    GeometryStrainRenderer::~GeometryStrainRenderer() {

    }

    void GeometryStrainRenderer::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/vis/glsl/geometrystrainrenderer.frag", "");
        if (_shader != 0) {
            _shader->setAttributeLocation(0, "in_Position");
        }
    }

    void GeometryStrainRenderer::deinit() {
        ShdrMgr.dispose(_shader);
        _shader = 0;
        VisualizationProcessor::deinit();
    }

    void GeometryStrainRenderer::updateResult(DataContainer& data) {
        ScopedTypedData<GeometryData> proxyGeometry(data, p_geometryID.getValue());
        ImageRepresentationGL::ScopedRepresentation strainData(data, p_strainId.getValue());
        ScopedTypedData<LightSourceData> light(data, p_lightId.getValue());

        if (proxyGeometry != 0 && strainData != 0 && _shader != 0) {
            if (p_enableShading.getValue() == false || light != nullptr) {
                // set modelview and projection matrices
                FramebufferActivationGuard fag(this);
                createAndAttachColorTexture();
                createAndAttachDepthTexture();

                _shader->activate();
                if (p_enableShading.getValue() && light != nullptr) {
                    light->bind(_shader, "_lightSource");
                }
                _shader->setUniform("_projectionMatrix", p_camera.getValue().getProjectionMatrix());
                _shader->setUniform("_viewMatrix", p_camera.getValue().getViewMatrix());
                _shader->setUniform("_color", p_color.getValue());

                tgt::TextureUnit strainUnit;
                strainData->bind(_shader, strainUnit, "_strainTexture");

                glEnable(GL_DEPTH_TEST);
                glDepthFunc(GL_LESS);
                glClearDepth(1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                proxyGeometry->render();

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

    std::string GeometryStrainRenderer::generateGlslHeader() const {
        std::string toReturn;
        if (p_enableShading.getValue())
            toReturn += "#define ENABLE_SHADING\n";
        return toReturn;
    }

    void GeometryStrainRenderer::updateShader() {
        _shader->setHeaders(generateGlslHeader());
        _shader->rebuild();

        validate(INVALID_SHADER);
    }

    void GeometryStrainRenderer::updateProperties(DataContainer& dataContainer) {
        p_lightId.setVisible(p_enableShading.getValue());
        validate(INVALID_PROPERTIES);
    }

}
