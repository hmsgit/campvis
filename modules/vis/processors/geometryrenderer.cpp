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

#include "geometryrenderer.h"

#include "tgt/glmath.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/renderdata.h"
#include "core/datastructures/meshgeometry.h"
#include "core/pipeline/processordecoratorshading.h"

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

    const std::string GeometryRenderer::loggerCat_ = "CAMPVis.modules.vis.GeometryRenderer";

    GeometryRenderer::GeometryRenderer(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_geometryID("geometryID", "Input Geometry ID", "gr.input", DataNameProperty::READ)
        , p_renderTargetID("p_renderTargetID", "Output Image", "gr.output", DataNameProperty::WRITE)
        , p_camera("camera", "Camera")
        , p_renderMode("RenderMode", "Render Mode", renderOptions, 7, AbstractProcessor::INVALID_RESULT | AbstractProcessor::INVALID_PROPERTIES)
        , p_useSolidColor("UseSolidColor", "Use Solid Color", true, AbstractProcessor::INVALID_RESULT | AbstractProcessor::INVALID_PROPERTIES)
        , p_solidColor("SolidColor", "Solid Color", tgt::vec4(1.f, .5f, 0.f, 1.f), tgt::vec4(0.f), tgt::vec4(1.f))
        , p_pointSize("PointSize", "Point Size", 3.f, .1f, 10.f)
        , p_lineWidth("LineWidth", "Line Width", 1.f, .1f, 10.f)
        , p_showWireframe("ShowWireframe", "Show Wireframe", true, AbstractProcessor::INVALID_RESULT | AbstractProcessor::INVALID_SHADER | AbstractProcessor::INVALID_PROPERTIES)
        , p_wireframeColor("WireframeColor", "Wireframe Color", tgt::vec4(1.f, 1.f, 1.f, 1.f), tgt::vec4(0.f), tgt::vec4(1.f))
        , _pointShader(0)
        , _meshShader(0)
    {
        addDecorator(new ProcessorDecoratorShading());

        addProperty(&p_geometryID);
        addProperty(&p_renderTargetID);
        addProperty(&p_camera);

        addProperty(&p_renderMode);

        addProperty(&p_useSolidColor);
        addProperty(&p_solidColor);

        addProperty(&p_pointSize);
        addProperty(&p_lineWidth);
        addProperty(&p_showWireframe);
        addProperty(&p_wireframeColor);

        decoratePropertyCollection(this);
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

        if (proxyGeometry != 0 && _pointShader != 0 && _meshShader != 0) {
            // select correct shader
            tgt::Shader* leShader = 0;
            if (p_renderMode.getOptionValue() == GL_POINTS || p_renderMode.getOptionValue() == GL_LINES || p_renderMode.getOptionValue() == GL_LINE_STRIP)
                leShader = _pointShader;
            else
                leShader = _meshShader;

            // calculate viewport matrix for NDC -> viewport conversion
            tgt::vec2 halfViewport = tgt::vec2(getEffectiveViewportSize()) / 2.f;
            tgt::mat4 viewportMatrix = tgt::mat4::createTranslation(tgt::vec3(halfViewport, 0.f)) * tgt::mat4::createScale(tgt::vec3(halfViewport, 1.f));

            // set modelview and projection matrices
            leShader->activate();
            leShader->setIgnoreUniformLocationError(true);
            decorateRenderProlog(data, leShader);
            leShader->setUniform("_projectionMatrix", p_camera.getValue().getProjectionMatrix());
            leShader->setUniform("_viewMatrix", p_camera.getValue().getViewMatrix());
            leShader->setUniform("_viewportMatrix", viewportMatrix);

            leShader->setUniform("_computeNormals", proxyGeometry->getNormalsBuffer() == 0);

            leShader->setUniform("_useSolidColor", p_useSolidColor.getValue());
            leShader->setUniform("_solidColor", p_solidColor.getValue());
            leShader->setUniform("_wireframeColor", p_wireframeColor.getValue());
            leShader->setUniform("_lineWidth", p_lineWidth.getValue());

            leShader->setUniform("_cameraPosition", p_camera.getValue().getPosition());
            leShader->setIgnoreUniformLocationError(false);

            FramebufferActivationGuard fag(this);
            createAndAttachColorTexture();
            createAndAttachDepthTexture();

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

            decorateRenderEpilog(leShader);
            leShader->deactivate();
            glDisable(GL_DEPTH_TEST);
            LGL_ERROR;

            data.addData(p_renderTargetID.getValue(), new RenderData(_fbo));
        }
        else {
            LERROR("No suitable input geometry found.");
        }

        validate(INVALID_RESULT);
    }

    std::string GeometryRenderer::generateGlslHeader(bool hasGeometryShader) const {
        std::string toReturn = getDecoratedHeader();

        if (hasGeometryShader && p_showWireframe.getValue())
            toReturn += "#define WIREFRAME_RENDERING\n";

        if (hasGeometryShader)
            toReturn += "#define HAS_GEOMETRY_SHADER\n";

        return toReturn;
    }

    void GeometryRenderer::updateShader() {
        _pointShader->setHeaders(generateGlslHeader(false));
        _pointShader->rebuild();
        _meshShader->setHeaders(generateGlslHeader(true));
        _meshShader->rebuild();
        validate(INVALID_SHADER);
    }

    void GeometryRenderer::updateProperties(DataContainer& dataContainer) {
        p_solidColor.setVisible(p_useSolidColor.getValue());

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
