#include "visualizationprocessor.h"

namespace TUMVis {

    VisualizationProcessor::VisualizationProcessor(GenericProperty<tgt::ivec2>& canvasSize)
        : AbstractProcessor()
        , _canvasSize("canvasSize", "Canvas Size", canvasSize.getValue())
    {
        canvasSize.addSharedProperty(&_canvasSize);
    }

    VisualizationProcessor::~VisualizationProcessor() {

    }

}
