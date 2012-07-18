#include "abstractprocessor.h"

namespace TUMVis {

    const std::string AbstractProcessor::loggerCat_ = "TUMVis.core.datastructures.Processor";


    AbstractProcessor::AbstractProcessor() {
    }

    AbstractProcessor::~AbstractProcessor() {

    }

    const InvalidationLevel& AbstractProcessor::getInvalidationLevel() const {
        return _invalidationLevel;
    }

    void AbstractProcessor::applyInvalidationLevel(InvalidationLevel il) {
        _invalidationLevel.setLevel(il);

        // If processor is no longer valid, notify observers
        if (! _invalidationLevel.isValid()) {
            notifyObservers(ProcessorObserverArgs(this, _invalidationLevel));
        }
    }

    void AbstractProcessor::onNotify(const PropertyObserverArgs& poa) {
        applyInvalidationLevel(poa._invalidationLevel);
    }

    PropertyCollection& AbstractProcessor::getPropertyCollection() {
        return _properties;
    }

    void AbstractProcessor::init() {

    }

}
