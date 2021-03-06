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

#include "fiberrenderer.h"

#include "cgt/glmath.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"


#include "modules/dti/datastructures/fiberdata.h"
#include "core/datastructures/cameradata.h"
#include "core/datastructures/lightsourcedata.h"
#include "core/datastructures/renderdata.h"
#include "core/datastructures/meshgeometry.h"

namespace campvis {
namespace dti {

    static const GenericOption<FiberRenderer::RenderMode> renderModeOptions[2] = {
        GenericOption<FiberRenderer::RenderMode>("Stripes", "Stripes", FiberRenderer::STRIPES), 
        GenericOption<FiberRenderer::RenderMode>("Tubes", "Tubes", FiberRenderer::TUBES)
    };

    static const GenericOption<FiberRenderer::ColoringMode> coloringModeOptions[2] = {
        GenericOption<FiberRenderer::ColoringMode>("Tangent", "Tangent", FiberRenderer::TANGENT), 
        GenericOption<FiberRenderer::ColoringMode>("ViewDependent", "View Dependent", FiberRenderer::VIEW_DEPENDENT)
    };

    const std::string FiberRenderer::loggerCat_ = "CAMPVis.modules.vis.FiberRenderer";

    FiberRenderer::FiberRenderer(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_strainId("StrainDataId", "Input Strain Data ID", "gr.strain", DataNameProperty::READ)
        , p_camera("Camera", "Camera ID", "camera", DataNameProperty::READ)
        , p_renderTargetID("p_renderTargetID", "Output Image", "gr.output", DataNameProperty::WRITE)
        , p_renderMode("RenderMode", "Render Mode", renderModeOptions, 2)
        , p_coloringMode("ColoringMode", "Coloring Mode", coloringModeOptions, 2)
        , p_lineWidth("LineWidth", "Line width", 2.f, .1f, 10.f, 0.1f)
        , p_enableShading("EnableShading", "Enable Shading", true)
        , p_lightId("LightId", "Input Light Source", "lightsource", DataNameProperty::READ)
        , _shader(0)
    {
        addProperty(p_strainId);
        addProperty(p_renderTargetID);
        addProperty(p_camera);
        addProperty(p_renderMode, INVALID_RESULT | INVALID_SHADER);
        addProperty(p_coloringMode, INVALID_RESULT | INVALID_PROPERTIES);

        addProperty(p_lineWidth);

        addProperty(p_enableShading, INVALID_RESULT | INVALID_PROPERTIES | INVALID_SHADER);
        addProperty(p_lightId);
    }

    FiberRenderer::~FiberRenderer() {

    }

    void FiberRenderer::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.load("modules/dti/glsl/fiberrenderer.vert", "modules/dti/glsl/fiberrenderer.geom", "modules/dti/glsl/fiberrenderer.frag", generateGlslHeader());
        invalidate(AbstractProcessor::INVALID_SHADER);
    }

    void FiberRenderer::deinit() {
        ShdrMgr.dispose(_shader);
        _shader = 0;
        VisualizationProcessor::deinit();
    }

    void FiberRenderer::updateResult(DataContainer& data) {
        ScopedTypedData<FiberData> strainData(data, p_strainId.getValue());
        ScopedTypedData<LightSourceData> light(data, p_lightId.getValue());
        ScopedTypedData<CameraData> camera(data, p_camera.getValue());

        if (strainData != nullptr && camera != nullptr && _shader != nullptr) {
            if (p_enableShading.getValue() == false || light != nullptr) {
                const cgt::Camera& cam = camera->getCamera();

                // set modelview and projection matrices
                FramebufferActivationGuard fag(this);
                createAndAttachColorTexture();
                createAndAttachDepthTexture();

                _shader->activate();
                _shader->setIgnoreUniformLocationError(true);
                if (p_enableShading.getValue() && light != nullptr) {
                    light->bind(_shader, "_lightSource");
                }
                _shader->setUniform("_projectionMatrix", cam.getProjectionMatrix());
                _shader->setUniform("_viewMatrix", cam.getViewMatrix());
                _shader->setUniform("_cameraPosition", cam.getPosition());
                _shader->setUniform("_fiberWidth", p_lineWidth.getValue()/4.f);
                _shader->setUniform("_coloringMode", p_coloringMode.getValue());
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
    }

    std::string FiberRenderer::generateGlslHeader() const {
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

    void FiberRenderer::updateShader() {
        _shader->setHeaders(generateGlslHeader());
        _shader->rebuild();
    }

    void FiberRenderer::updateProperties(DataContainer& dataContainer) {
        p_lightId.setVisible(p_enableShading.getValue());
    }

}
}
