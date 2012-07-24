#include "visualizationprocessor.h"

namespace TUMVis {

    VisualizationProcessor::VisualizationProcessor(GenericProperty<tgt::ivec2>& canvasSize)
        : AbstractProcessor()
        , _renderTargetSize("canvasSize", "Canvas Size", canvasSize.getValue())
    {
        canvasSize.addSharedProperty(&_renderTargetSize);
        _renderTargetSize.addObserver(this);
    }

    VisualizationProcessor::~VisualizationProcessor() {

    }

}
