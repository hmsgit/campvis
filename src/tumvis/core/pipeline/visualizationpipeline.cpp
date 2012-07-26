#include "visualizationpipeline.h"
#include "tgt/camera.h"
#include "tgt/quadrenderer.h"
#include "tgt/quadric.h"
#include "tgt/glcontext.h"
#include "core/datastructures/imagedatarendertarget.h"

namespace TUMVis {
    const std::string VisualizationPipeline::loggerCat_ = "TUMVis.core.datastructures.VisualizationPipeline";

    VisualizationPipeline::VisualizationPipeline() 
        : AbstractPipeline()
        , tgt::EventListener()
        , _renderTargetSize("canvasSize", "Canvas Size", tgt::ivec2(128, 128))
        , _renderTargetID("renderTargetID", "Render Target ID", "VisualizationPipeline.renderTarget")
        , _canvas(0)
    {
        _data.s_dataAdded.connect(this, &VisualizationPipeline::onDataContainerDataAdded);
    }

    VisualizationPipeline::~VisualizationPipeline() {
        _data.s_dataAdded.disconnect(this);
    }

    void VisualizationPipeline::onEvent(tgt::Event* e) {
        // cycle through event handlers, ask each one if it handles the event and if so, execute it.
        for (std::vector<AbstractEventHandler*>::iterator it = _eventHandlers.begin(); it != _eventHandlers.end() && e->isAccepted(); ++it) {
            if ((*it)->accept(e)) {
                (*it)->execute(e);
            }
        }

        if (e->isAccepted()) {
            EventListener::onEvent(e);
        }
    }

    PropertyCollection& VisualizationPipeline::getPropertyCollection() {
        return _properties;
    }

    void VisualizationPipeline::init() {
        tgtAssert(_canvas != 0, "Set a valid canvas before calling this method!");
        tgt::GLContextScopedLock lock(_canvas->getContext());
        AbstractPipeline::init();
    }

    void VisualizationPipeline::deinit() {
        tgtAssert(_canvas != 0, "Set a valid canvas before calling this method!");
        tgt::GLContextScopedLock lock(_canvas->getContext());
        AbstractPipeline::deinit();
    }

    const tgt::ivec2& VisualizationPipeline::getRenderTargetSize() const {
        return _renderTargetSize.getValue();
    }

    void VisualizationPipeline::onDataContainerDataAdded(const std::string& name, const DataHandle* dh) {
        if (name == _renderTargetID.getValue()) {
            s_renderTargetChanged();
        }
    }

    const std::string& VisualizationPipeline::getRenderTargetID() const {
        return _renderTargetID.getValue();
    }

    void VisualizationPipeline::setRenderTargetSize(const tgt::ivec2& size) {
        _renderTargetSize.setValue(size);
    }

    void VisualizationPipeline::lockGLContextAndExecuteProcessor(AbstractProcessor& processor) {
        tgtAssert(_canvas != 0, "Set a valid canvas before calling this method!");
        tgt::GLContextScopedLock lock(_canvas->getContext());
        executeProcessor(processor);
        glFlush();  // TODO: is glFlush enough or do we need a glFinish here?
    }

    void VisualizationPipeline::setCanvas(tgt::GLCanvas* canvas) {
        _canvas = canvas;
    }

}
