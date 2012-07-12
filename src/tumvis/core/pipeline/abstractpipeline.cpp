#include "abstractpipeline.h"

namespace TUMVis {
    const std::string AbstractPipeline::loggerCat_ = "TUMVis.core.datastructures.AbstractPipeline";

    AbstractPipeline::AbstractPipeline() 
    {
    }

    AbstractPipeline::~AbstractPipeline() {
    }

    PropertyCollection& AbstractPipeline::getPropertyCollection() {
        return _properties;
    }

    void AbstractPipeline::init() {
        // initialize all processors:
//         for (std::vector<AbstractProcessor*>::iterator it = _processors.begin(); it != _processors.end(); ++it) {
//             (*it)->initGL();
//         }
    }

}
