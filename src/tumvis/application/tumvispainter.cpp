#include "tumvispainter.h"

#include "tgt/assert.h"
#include "tgt/camera.h"
#include "tgt/qt/qtthreadedcanvas.h"
#include "tgt/qt/qtglcontext.h"
#include "tgt/qt/qtcontextmanager.h"
#include "tgt/quadrenderer.h"
#include "tgt/quadric.h"

#include "core/datastructures/imagedatarendertarget.h"
#include "core/pipeline/visualizationpipeline.h"

namespace TUMVis {
    const std::string TumVisPainter::loggerCat_ = "TUMVis.core.TumVisPainter";

    TumVisPainter::TumVisPainter(tgt::QtThreadedCanvas* canvas, VisualizationPipeline* pipeline)
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
            getCanvas()->getContext()->acquire();
            paint();
            getCanvas()->swap();

            while (!_stopExecution && !_dirty)
                _renderCondition.wait(lock);
        }

        // release OpenGL context, so that other threads can access it
        CtxtMgr.releaseCurrentContext();
    }

    void TumVisPainter::paint() {
        if (getCanvas() == 0)
            return;

        while (_dirty) {
            _dirty = false;

            const tgt::ivec2& size = getCanvas()->getSize();
            glViewport(0, 0, size.x, size.y);

            // try get Data
            DataContainer::ScopedTypedData<ImageDataRenderTarget> image(_pipeline->getDataContainer(), _pipeline->getRenderTargetID());
            //const ImageDataRenderTarget* image = _pipeline->getRenderTarget();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            if (image != 0) {
                // activate shader
                _copyShader->activate();
                _copyShader->setIgnoreUniformLocationError(true);
                _copyShader->setUniform("_viewportSize", size);
                _copyShader->setUniform("_viewportSizeRCP", 1.f / tgt::vec2(size));
                _copyShader->setIgnoreUniformLocationError(false);

                // bind input textures
                tgt::TextureUnit colorUnit, depthUnit;
                image->bind(_copyShader, colorUnit, depthUnit);
                LGL_ERROR;

                // execute the shader
                tgt::QuadRenderer::renderQuad();
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
        }
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

    void TumVisPainter::onRenderTargetChanged() {
        // TODO:    What happens, if the mutex is still acquired?
        //          Will the render thread woken up as soon as it is released?
        if (!_stopExecution) {
            _dirty = true;
            _renderCondition.notify_all();
        }
    }

    void TumVisPainter::setCanvas(tgt::GLCanvas* canvas) {
        tgtAssert(dynamic_cast<tgt::QtThreadedCanvas*>(canvas) != 0, "Canvas must be of type QtThreadedCanvas!");
        Painter::setCanvas(canvas);
    }
}