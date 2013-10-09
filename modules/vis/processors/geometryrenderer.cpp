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
    const std::string GeometryRenderer::loggerCat_ = "CAMPVis.modules.vis.GeometryRenderer";

    GeometryRenderer::GeometryRenderer(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_geometryID("geometryID", "Input Geometry ID", "gr.input", DataNameProperty::READ)
        , p_renderTargetID("p_renderTargetID", "Output Image", "gr.output", DataNameProperty::WRITE)
        , p_camera("camera", "Camera")
        , p_color("color", "Rendering Color", tgt::vec4(1.f), tgt::vec4(0.f), tgt::vec4(1.f))
        , _shader(0)
    {
        addDecorator(new ProcessorDecoratorShading());

        addProperty(&p_geometryID);
        addProperty(&p_renderTargetID);
        addProperty(&p_camera);
        addProperty(&p_color);

        decoratePropertyCollection(this);
    }

    GeometryRenderer::~GeometryRenderer() {

    }

    void GeometryRenderer::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/vis/glsl/geometryrenderer.frag", "", false);
        if (_shader != 0) {
            _shader->setAttributeLocation(0, "in_Position");
        }
    }

    void GeometryRenderer::deinit() {
        ShdrMgr.dispose(_shader);
        _shader = 0;
        VisualizationProcessor::deinit();
    }

    void GeometryRenderer::process(DataContainer& data) {
        ScopedTypedData<GeometryData> proxyGeometry(data, p_geometryID.getValue());

        if (proxyGeometry != 0 && _shader != 0) {
            if (hasInvalidShader()) {
                _shader->setHeaders(generateGlslHeader());
                _shader->rebuild();
                validate(INVALID_SHADER);
            }

            // set modelview and projection matrices
            _shader->activate();
            _shader->setIgnoreUniformLocationError(true);
            decorateRenderProlog(data, _shader);
            _shader->setUniform("_projectionMatrix", p_camera.getValue().getProjectionMatrix());
            _shader->setUniform("_viewMatrix", p_camera.getValue().getViewMatrix());
            _shader->setUniform("_color", p_color.getValue());
            _shader->setUniform("_cameraPosition", p_camera.getValue().getPosition());
            _shader->setIgnoreUniformLocationError(false);

            // create entry points texture
            FramebufferActivationGuard fag(this);
            createAndAttachColorTexture();
            createAndAttachDepthTexture();

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_LESS);
            glClearDepth(1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            proxyGeometry->render();

            decorateRenderEpilog(_shader);
            _shader->deactivate();
            glDisable(GL_DEPTH_TEST);
            LGL_ERROR;

            data.addData(p_renderTargetID.getValue(), new RenderData(_fbo));
        }
        else {
            LERROR("No suitable input geometry found.");
        }

        validate(INVALID_RESULT);
    }

    std::string GeometryRenderer::generateGlslHeader() const {
        std::string toReturn = getDecoratedHeader();
        return toReturn;
    }

}
