#include "visualizationpipeline.h"
#include "tgt/camera.h"
#include "tgt/quadrenderer.h"
#include "tgt/quadric.h"
#include "core/datastructures/imagedatarendertarget.h"

namespace TUMVis {
    const std::string VisualizationPipeline::loggerCat_ = "TUMVis.core.datastructures.VisualizationPipeline";

    VisualizationPipeline::VisualizationPipeline(tgt::GLCanvas* canvas /*= 0*/) 
        : AbstractPipeline()
        , _canvas(0)
        , _canvasSize("canvasSize", "Canvas Size", tgt::ivec2(128, 128))
        , _renderTargetID("renderTargetID", "Render Target ID", "VisualizationPipeline.renderTarget")
    {
        setCanvas(canvas);
    }

    VisualizationPipeline::~VisualizationPipeline() {
    }

    void VisualizationPipeline::onEvent(tgt::Event* e) {
        // cycle through event handlers, ask each one if it handles the event and if so, execute it.
        for (std::vector<AbstractEventHandler*>::iterator it = _eventHandlers.begin(); it != _eventHandlers.end() && !e->isAccepted(); ++it) {
            if ((*it)->accept(e)) {
                (*it)->execute(e);
            }
        }

        if (e->isAccepted()) {
            EventListener::onEvent(e);
        }
    }

    void VisualizationPipeline::setCanvas(tgt::GLCanvas* canvas) {
        if (_canvas != 0 && _canvas->getEventHandler() != 0) {
            _canvas->getEventHandler()->removeListener(this);
        }
        _canvas = canvas;
        _canvasSize.setValue(_canvas->getSize());
        if (_canvas->getEventHandler() != 0) {
            _canvas->getEventHandler()->addListenerToFront(this);
        }
    }

    PropertyCollection& VisualizationPipeline::getPropertyCollection() {
        return _properties;
    }

    void VisualizationPipeline::init() {
        AbstractPipeline::init();

        try {
            // TODO:    Remove hardcoded paths, and use ShdrMgr.addPath() at some central location
            _copyShader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "core/glsl/copyimage.frag", "", false);
        }
        catch (tgt::Exception& e) {
            LERRORC("main.cpp", "Encountered tgt::Exception: " << e.what());
        }
    }

    void VisualizationPipeline::paint() {
        execute();

        if (_canvas == 0)
            return;

        // get OpenGL focus and setup context
        _canvas->getGLFocus();
        glViewport(0, 0, _canvas->getSize().x, _canvas->getSize().y);

        // try get Data
        const ImageDataRenderTarget* image = _data.getTypedData<ImageDataRenderTarget>(_renderTargetID.getValue());
        if (image != 0) {
            
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            // activate shader
            _copyShader->activate();
            _copyShader->setIgnoreUniformLocationError(true);
            _copyShader->setUniform("_viewportSize", tgt::vec2(_canvasSize.getValue()));
            _copyShader->setUniform("_viewportSizeRCP", 1.f / tgt::vec2(_canvasSize.getValue()));
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
            glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);  
            getCamera()->setPosition(tgt::vec3(0.f,0.f,2.f)); 
            getCamera()->look();  
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

    void VisualizationPipeline::sizeChanged(const tgt::ivec2& size) {
        // nothing to change if nothing changes
        if (size != _canvasSize.getValue()) {
            _canvasSize.setValue(size);
            // this should be enough here: Changing the property issues a notification of the 
            // owner (i.e. this pipeline), who is then up to decide how to proceed.
        }
    }

    const tgt::ivec2& VisualizationPipeline::getCanvasSize() const {
        return _canvasSize.getValue();
    }

}
