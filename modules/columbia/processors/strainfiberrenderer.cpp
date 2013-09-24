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

#include "strainfiberrenderer.h"

#include "tgt/glmath.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"


#include "modules/columbia/datastructures/fiberdata.h"
#include "core/datastructures/renderdata.h"
#include "core/datastructures/meshgeometry.h"
#include "core/pipeline/processordecoratorshading.h"

namespace campvis {

    static const GenericOption<StrainFiberRenderer::RenderMode> renderModeOptions[2] = {
        GenericOption<StrainFiberRenderer::RenderMode>("Stripes", "Stripes", StrainFiberRenderer::STRIPES), 
        GenericOption<StrainFiberRenderer::RenderMode>("Tubes", "Tubes", StrainFiberRenderer::TUBES)
    };

    const std::string StrainFiberRenderer::loggerCat_ = "CAMPVis.modules.vis.StrainFiberRenderer";

    StrainFiberRenderer::StrainFiberRenderer(IVec2Property& canvasSize)
        : VisualizationProcessor(canvasSize)
        , p_strainId("StrainDataId", "Input Strain Data ID", "gr.strain", DataNameProperty::READ)
        , p_renderTargetID("p_renderTargetID", "Output Image", "gr.output", DataNameProperty::WRITE)
        , p_camera("Camera", "Camera ID")//, "camera", DataNameProperty::READ, AbstractProcessor::INVALID_RESULT, DataNameProperty::CameraData)
        , p_lineWidth("LineWidth", "Line width", 3.f, .5f, 10.f, 0.1f)
        , p_renderMode("RenderMode", "Render Mode", renderModeOptions, 2, AbstractProcessor::INVALID_RESULT | AbstractProcessor::INVALID_SHADER)
        , p_color("color", "Rendering Color", tgt::vec4(1.f), tgt::vec4(0.f), tgt::vec4(1.f))
        , _shader(0)
    {
        addDecorator(new ProcessorDecoratorShading());

        addProperty(&p_strainId);
        addProperty(&p_renderTargetID);
        addProperty(&p_camera);
        addProperty(&p_color);
        addProperty(&p_renderMode);
        addProperty(&p_lineWidth);

        decoratePropertyCollection(this);
    }

    StrainFiberRenderer::~StrainFiberRenderer() {

    }

    void StrainFiberRenderer::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("modules/columbia/glsl/strainfiberrenderer.vert", "modules/columbia/glsl/strainfiberrenderer.geom", "modules/columbia/glsl/strainfiberrenderer.frag", "", false);
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

    void StrainFiberRenderer::process(DataContainer& data) {
        if (hasInvalidShader()) {
            _shader->setHeaders(generateGlslHeader());
            _shader->rebuild();
            validate(INVALID_SHADER);
        }
        
        DataContainer::ScopedTypedData<FiberData> strainData(data, p_strainId.getValue());
        if (strainData != 0 && _shader != 0) {
            const tgt::Camera& camera = p_camera.getValue();

            // set modelview and projection matrices
            FramebufferActivationGuard fag(this);
            createAndAttachColorTexture();
            createAndAttachDepthTexture();

            _shader->activate();
            _shader->setIgnoreUniformLocationError(true);
            decorateRenderProlog(data, _shader);
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

            decorateRenderEpilog(_shader);
            _shader->deactivate();
            glDisable(GL_DEPTH_TEST);
            LGL_ERROR;

            data.addData(p_renderTargetID.getValue(), new RenderData(_fbo));
            p_renderTargetID.issueWrite();
        }
        else {
            LERROR("No suitable input geometry found.");
        }

        validate(INVALID_RESULT);
    }

    std::string StrainFiberRenderer::generateGlslHeader() const {
        std::string toReturn = getDecoratedHeader();

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

}
