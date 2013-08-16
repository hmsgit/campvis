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

#include "campvispainter.h"

#include "tgt/assert.h"
#include "tgt/camera.h"
#include "tgt/quadric.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"
#include "tgt/qt/qtthreadedcanvas.h"
#include "tgt/qt/qtglcontext.h"
#include "tgt/qt/qtcontextmanager.h"


#include "core/datastructures/imagedata.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/visualizationpipeline.h"
#include "core/tools/job.h"
#include "core/tools/opengljobprocessor.h"
#include "core/tools/quadrenderer.h"

namespace campvis {
    const std::string TumVisPainter::loggerCat_ = "CAMPVis.core.TumVisPainter";

    TumVisPainter::TumVisPainter(tgt::GLCanvas* canvas, VisualizationPipeline* pipeline)
        : Runnable()
        , tgt::Painter(canvas)
        , _pipeline(0)
        , _copyShader(0)
    {
        tgtAssert(getCanvas() != 0, "The given canvas must not be 0!");
        _dirty = true;
        setPipeline(pipeline);
    }

    TumVisPainter::~TumVisPainter() {

    }

    void TumVisPainter::stop() {
        // we need to execute run() one more time to ensure correct release of the OpenGL context
        _stopExecution = true;
        _renderCondition.notify_all();

        Runnable::stop();
    }

    void TumVisPainter::run() {
        std::unique_lock<tbb::mutex> lock(CtxtMgr.getGlMutex());

        while (! _stopExecution) {

            /*getCanvas()->getContext()->acquire();
            paint();
            getCanvas()->swap();*/

            //while (!_stopExecution)
                _renderCondition.wait(lock);
        }

        // release OpenGL context, so that other threads can access it
        CtxtMgr.releaseCurrentContext();
    }

    void TumVisPainter::paint() {
        if (getCanvas() == 0)
            return;

        const tgt::ivec2& size = getCanvas()->getSize();
        glViewport(0, 0, size.x, size.y);

        // try get Data
        DataContainer::ScopedTypedData<RenderData> rd(_pipeline->getDataContainer(), _pipeline->getRenderTargetID());
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        if (rd != 0) {
            // activate shader
            _copyShader->activate();
            _copyShader->setIgnoreUniformLocationError(true);
            _copyShader->setUniform("_viewportSize", size);
            _copyShader->setUniform("_viewportSizeRCP", 1.f / tgt::vec2(size));
            _copyShader->setIgnoreUniformLocationError(false);

            // bind input textures
            tgt::TextureUnit colorUnit, depthUnit;
            rd->bind(_copyShader, colorUnit, depthUnit);
            LGL_ERROR;

            // execute the shader
            _copyShader->setAttributeLocation(0, "in_Position");
            _copyShader->setAttributeLocation(1, "in_TexCoords");
            QuadRdr.renderQuad();
            _copyShader->deactivate();
            LGL_ERROR;
        }
        else {
            // TODO: render some nifty error texture
            //       so long, we do some dummy rendering
            tgt::Camera c(tgt::vec3(0.f,0.f,2.f)); 
            c.look();  
            glColor3f(1.f, 0.f, 0.f);  
            tgt::Sphere sphere(.5f, 64, 32);  
            sphere.render();

            /*
            // render error texture
            if (!errorTex_) {
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                return;
            }
            glClear(GL_DEPTH_BUFFER_BIT);

            glActiveTexture(GL_TEXTURE0);
            errorTex_->bind();
            errorTex_->enable();

            glColor3f(1.f, 1.f, 1.f);
            renderQuad();

            errorTex_->disable();*/
        }
        LGL_ERROR;

        getCanvas()->swap();
    }

    void TumVisPainter::sizeChanged(const tgt::ivec2& size) {
        _pipeline->setRenderTargetSize(size);
    }

    void TumVisPainter::init() {
        try {
            // TODO:    Remove hardcoded paths, and use ShdrMgr.addPath() at some central location
            _copyShader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "core/glsl/copyimage.frag", "", false);
        }
        catch (tgt::Exception& e) {
            LERRORC("main.cpp", "Encountered tgt::Exception: " << e.what());
        }
    }

    void TumVisPainter::deinit() {
        ShdrMgr.dispose(_copyShader);

        if (_pipeline != 0) {
            _pipeline->s_renderTargetChanged.disconnect(this);
            if (getCanvas()->getEventHandler() != 0)
                getCanvas()->getEventHandler()->removeListener(_pipeline);
            _pipeline = 0;
        }
    }

    void TumVisPainter::setPipeline(VisualizationPipeline* pipeline) {
        tgtAssert(pipeline != 0, "The given pipeline must not be 0.");
        if (_pipeline != 0) {
            _pipeline->s_renderTargetChanged.disconnect(this);
            if (getCanvas()->getEventHandler() != 0)
                getCanvas()->getEventHandler()->removeListener(_pipeline);
        }

        _pipeline = pipeline;
        _pipeline->s_renderTargetChanged.connect(this, &TumVisPainter::onRenderTargetChanged);
        _pipeline->setRenderTargetSize(getCanvas()->getSize());
        if (getCanvas()->getEventHandler() != 0)
            getCanvas()->getEventHandler()->addListenerToFront(_pipeline);
    }

    void TumVisPainter::repaint() {
        GLJobProc.enqueueJob(getCanvas(), makeJobOnHeap(this, &TumVisPainter::paint), OpenGLJobProcessor::PaintJob);
    }

    void TumVisPainter::onRenderTargetChanged() {
        repaint();
    }

    void TumVisPainter::setCanvas(tgt::GLCanvas* canvas) {
        tgtAssert(dynamic_cast<tgt::QtThreadedCanvas*>(canvas) != 0, "Canvas must be of type QtThreadedCanvas!");
        Painter::setCanvas(canvas);
    }
}
