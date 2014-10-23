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

#include "geometryrenderer.h"

#include "cgt/glmath.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/lightsourcedata.h"
#include "core/datastructures/renderdata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/meshgeometry.h"

namespace campvis {
    static const GenericOption<GLenum> renderOptions[7] = {
        GenericOption<GLenum>("points", "GL_POINTS", GL_POINTS),
        GenericOption<GLenum>("lines", "GL_LINES", GL_LINES),
        GenericOption<GLenum>("linestrip", "GL_LINE_STRIP", GL_LINE_STRIP),
        GenericOption<GLenum>("triangles", "GL_TRIANGLES", GL_TRIANGLES),
        GenericOption<GLenum>("trianglefan", "GL_TRIANGLE_FAN", GL_TRIANGLE_FAN),
        GenericOption<GLenum>("trianglestrip", "GL_TRIANGLE_STRIP", GL_TRIANGLE_STRIP),
        GenericOption<GLenum>("polygon", "GL_POLYGON", GL_POLYGON)
    };

    static const GenericOption<GeometryRenderer::ColoringMode> coloringOptions[3] = {
        GenericOption<GeometryRenderer::ColoringMode>("GeometryColor", "Original Geometry Color", GeometryRenderer::GEOMETRY_COLOR),
        GenericOption<GeometryRenderer::ColoringMode>("SolidColor", "Solid Color", GeometryRenderer::SOLID_COLOR),
        GenericOption<GeometryRenderer::ColoringMode>("TextureColor", "Color from Texture Lookup", GeometryRenderer::TEXTURE_COLOR),
    };

    const std::string GeometryRenderer::loggerCat_ = "CAMPVis.modules.vis.GeometryRenderer";

    GeometryRenderer::GeometryRenderer(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_geometryID("geometryID", "Input Geometry ID", "gr.input", DataNameProperty::READ)
        , p_textureID("TextureId", "Input Texture ID (optional)", "gr.inputtexture", DataNameProperty::READ)
        , p_renderTargetID("p_renderTargetID", "Output Image", "gr.output", DataNameProperty::WRITE)
        , p_camera("camera", "Camera")
        , p_enableShading("EnableShading", "Enable Shading", true)
        , p_lightId("LightId", "Input Light Source", "lightsource", DataNameProperty::READ)
        , p_renderMode("RenderMode", "Render Mode", renderOptions, 7)
        , p_coloringMode("ColoringMode", "ColoringMode", coloringOptions, 3)
        , p_solidColor("SolidColor", "Solid Color", tgt::vec4(1.f, .5f, 0.f, 1.f), tgt::vec4(0.f), tgt::vec4(1.f))
        , p_pointSize("PointSize", "Point Size", 3.f, .1f, 10.f)
        , p_lineWidth("LineWidth", "Line Width", 1.f, .1f, 10.f)
        , p_showWireframe("ShowWireframe", "Show Wireframe", true)
        , p_wireframeColor("WireframeColor", "Wireframe Color", tgt::vec4(1.f, 1.f, 1.f, 1.f), tgt::vec4(0.f), tgt::vec4(1.f))
        , _pointShader(0)
        , _meshShader(0)
    {
        p_coloringMode.selectByOption(SOLID_COLOR);

        addProperty(p_geometryID);
        addProperty(p_textureID);
        addProperty(p_renderTargetID);
        addProperty(p_camera);

        addProperty(p_enableShading, INVALID_RESULT | INVALID_PROPERTIES | INVALID_SHADER);
        addProperty(p_lightId);

        addProperty(p_renderMode, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_coloringMode, INVALID_RESULT | INVALID_SHADER | INVALID_PROPERTIES);
        addProperty(p_solidColor);

        addProperty(p_pointSize);
        addProperty(p_lineWidth);
        addProperty(p_showWireframe, INVALID_RESULT | INVALID_SHADER | INVALID_PROPERTIES);
        addProperty(p_wireframeColor);
    }

    GeometryRenderer::~GeometryRenderer() {

    }

    void GeometryRenderer::init() {
        VisualizationProcessor::init();
        _pointShader = ShdrMgr.load("modules/vis/glsl/geometryrenderer.vert", "modules/vis/glsl/geometryrenderer.frag", generateGlslHeader(false));
        _meshShader = ShdrMgr.load("modules/vis/glsl/geometryrenderer.vert", "modules/vis/glsl/geometryrenderer.geom", "modules/vis/glsl/geometryrenderer.frag", generateGlslHeader(true));
    }

    void GeometryRenderer::deinit() {
        ShdrMgr.dispose(_pointShader);
        _pointShader = 0;
        ShdrMgr.dispose(_meshShader);
        _meshShader = 0;

        VisualizationProcessor::deinit();
    }

    void GeometryRenderer::updateResult(DataContainer& data) {
        ScopedTypedData<GeometryData> proxyGeometry(data, p_geometryID.getValue());
        ScopedTypedData<LightSourceData> light(data, p_lightId.getValue());
        ScopedTypedData<RenderData> rd(data, p_textureID.getValue());
        ImageRepresentationGL::ScopedRepresentation repGl(data, p_textureID.getValue());

        const ImageRepresentationGL* texture = nullptr;
        if (p_coloringMode.getOptionValue() == TEXTURE_COLOR) {
            if (proxyGeometry->hasTextureCoordinates()) {
                if (rd != nullptr && rd->getNumColorTextures() > 0)
                    texture = rd->getColorTexture()->getRepresentation<ImageRepresentationGL>();
                else if (repGl != nullptr)
                    texture = repGl;
            }
            else {
                LERROR("Cannot use textured rendering since input geometry has no texture coordinates!");
            }
        }

        if (proxyGeometry != 0 
            && (p_enableShading.getValue() == false || light != nullptr)
            && (p_coloringMode.getOptionValue() != TEXTURE_COLOR || texture != nullptr) 
            && _pointShader != 0 && _meshShader != 0)
            {
            // select correct shader
            tgt::Shader* leShader = 0;
            if (p_renderMode.getOptionValue() == GL_POINTS || p_renderMode.getOptionValue() == GL_LINES || p_renderMode.getOptionValue() == GL_LINE_STRIP)
                leShader = _pointShader;
            else
                leShader = _meshShader;

            // calculate viewport matrix for NDC -> viewport conversion
            tgt::vec2 halfViewport = tgt::vec2(getEffectiveViewportSize()) / 2.f;
            tgt::mat4 viewportMatrix = tgt::mat4::createTranslation(tgt::vec3(halfViewport, 0.f)) * tgt::mat4::createScale(tgt::vec3(halfViewport, 1.f));

            leShader->activate();

            // bind texture if needed
            tgt::TextureUnit tut, narf, blub, textureUnit;
            tut.activate();
            if (texture != nullptr)
                texture->bind(leShader, textureUnit, "_texture", "_textureParams");

            // set modelview and projection matrices
            leShader->setIgnoreUniformLocationError(true);
            
            if (p_enableShading.getValue() && light != nullptr) {
                light->bind(leShader, "_lightSource");
            }

            leShader->setUniform("_projectionMatrix", p_camera.getValue().getProjectionMatrix());
            leShader->setUniform("_viewMatrix", p_camera.getValue().getViewMatrix());
            leShader->setUniform("_viewportMatrix", viewportMatrix);

            leShader->setUniform("_computeNormals", proxyGeometry->getNormalsBuffer() == 0);

            leShader->setUniform("_coloringMode", p_coloringMode.getValue());
            leShader->setUniform("_solidColor", p_solidColor.getValue());
            leShader->setUniform("_wireframeColor", p_wireframeColor.getValue());
            leShader->setUniform("_lineWidth", p_lineWidth.getValue());

            leShader->setUniform("_cameraPosition", p_camera.getValue().getPosition());
            leShader->setIgnoreUniformLocationError(false);

            FramebufferActivationGuard fag(this);
            createAndAttachColorTexture();
            createAndAttachDepthTexture();

            static const GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 , GL_COLOR_ATTACHMENT2 };
            if (proxyGeometry->hasPickingInformation()) {
                createAndAttachColorTexture();
                glDrawBuffers(3, buffers);
            }

            glEnable(GL_BLEND);
            glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            glClearDepth(1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            if (p_renderMode.getOptionValue() == GL_POINTS)
                glPointSize(p_pointSize.getValue());
            else if (p_renderMode.getOptionValue() == GL_LINES || p_renderMode.getOptionValue() == GL_LINE_STRIP)
                glLineWidth(p_lineWidth.getValue());

            proxyGeometry->render(p_renderMode.getOptionValue());

            if (p_renderMode.getOptionValue() == GL_POINTS)
                glPointSize(1.f);
            else if (p_renderMode.getOptionValue() == GL_LINES || p_renderMode.getOptionValue() == GL_LINE_STRIP)
                glLineWidth(1.f);


            if (proxyGeometry->hasPickingInformation()) {
                glDrawBuffers(1, buffers);
            }

            leShader->deactivate();
            glDepthFunc(GL_LESS);
            glDisable(GL_DEPTH_TEST);
            glDisable(GL_BLEND);
            LGL_ERROR;

            data.addData(p_renderTargetID.getValue(), new RenderData(_fbo));
        }
        else {
            LDEBUG("No suitable input geometry found.");
        }
    }

    std::string GeometryRenderer::generateGlslHeader(bool hasGeometryShader) const {
        std::string toReturn;

        if (p_enableShading.getValue())
            toReturn += "#define ENABLE_SHADING\n";

        if (hasGeometryShader && p_showWireframe.getValue())
            toReturn += "#define WIREFRAME_RENDERING\n";

        if (hasGeometryShader)
            toReturn += "#define HAS_GEOMETRY_SHADER\n";

        if (p_coloringMode.getOptionValue() == TEXTURE_COLOR)
            toReturn += "#define ENABLE_TEXTURING\n";

        return toReturn;
    }

    void GeometryRenderer::updateShader() {
        _pointShader->setHeaders(generateGlslHeader(false));
        _pointShader->rebuild();
        _meshShader->setHeaders(generateGlslHeader(true));
        _meshShader->rebuild();
    }

    void GeometryRenderer::updateProperties(DataContainer& dataContainer) {
        p_solidColor.setVisible(p_coloringMode.getOptionValue() == SOLID_COLOR);
        p_lightId.setVisible(p_enableShading.getValue());

        switch (p_renderMode.getOptionValue()) {
            case GL_POINTS:
                p_pointSize.setVisible(true);
                p_lineWidth.setVisible(false);
                p_showWireframe.setVisible(false);
                break;
            case GL_LINES: // fallthrough
            case GL_LINE_STRIP:
                p_pointSize.setVisible(false);
                p_lineWidth.setVisible(true);
                p_showWireframe.setVisible(false);
                break;
            case GL_TRIANGLES: // fallthrough
            case GL_TRIANGLE_FAN: // fallthrough
            case GL_TRIANGLE_STRIP: // fallthrough
            case GL_POLYGON:
                p_pointSize.setVisible(false);
                p_lineWidth.setVisible(p_showWireframe.getValue());
                p_showWireframe.setVisible(true);
                break;
        }

        p_wireframeColor.setVisible(p_showWireframe.getValue());
    }

}
