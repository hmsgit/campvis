#include "visualizationpipeline.h"

namespace TUMVis {
    const std::string VisualizationPipeline::loggerCat_ = "TUMVis.core.datastructures.VisualizationPipeline";

    VisualizationPipeline::VisualizationPipeline(tgt::GLCanvas* canvas /*= 0*/) 
        : _canvas(canvas)
    {
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
    }

    void VisualizationPipeline::setCanvas(tgt::GLCanvas* canvas) {
        if (_canvas != 0 && _canvas->getEventHandler() != 0) {
            _canvas->getEventHandler()->removeListener(this);
        }
        _canvas = canvas;
        if (_canvas->getEventHandler() != 0) {
            _canvas->getEventHandler()->addListenerToFront(this);
        }
    }

    PropertyCollection& VisualizationPipeline::getPropertyCollection() {
        return _properties;
    }

    void VisualizationPipeline::init() {
        AbstractPipeline::init();
        // initialize all processors:
//         for (std::vector<AbstractProcessor*>::iterator it = _processors.begin(); it != _processors.end(); ++it) {
//             (*it)->initGL();
//         }
    }

    void VisualizationPipeline::paint() {
        // TODO:    implement - is there a generic way to initiate the painting in the subclasses?
    }

    void VisualizationPipeline::sizeChanged(const tgt::ivec2& size) {
        // nothing to change if nothing changes
        if (size != _canvasSize) {
            _canvasSize = size;
        }

        // TODO:    initiate pipeline evaluation, is a simple execute() what we want?
        execute();
    }

}
