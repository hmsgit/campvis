#include "abstractpipeline.h"

namespace TUMVis {

    const std::string AbstractPipeline::loggerCat_ = "TUMVis.core.datastructures.AbstractPipeline";

    void AbstractPipeline::onEvent(tgt::Event* e) {
        // cycle through event handlers, ask each one if it handles the event and if so, execute it.
        for (std::vector<AbstractEventHandler*>::iterator it = _eventHandlers.begin(); it != _eventHandlers.end() && !e->isAccepted(); ++it) {
            if ((*it)->accept(e)) {
                (*it)->execute(e);
            }
        }
    }

    void AbstractPipeline::setCanvas(tgt::GLCanvas* canvas) {
        _canvas = canvas;
    }
}
