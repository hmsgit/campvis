#include "abstractprocessor.h"

namespace TUMVis {

    const std::string AbstractProcessor::loggerCat_ = "TUMVis.core.datastructures.Processor";


    AbstractProcessor::AbstractProcessor()
        : HasPropertyCollection()
    {
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

    void AbstractProcessor::init() {
    }

    void AbstractProcessor::deinit() {
    }


    void AbstractProcessor::lockProperties() {
        _properties.lockAllProperties();
    }

    void AbstractProcessor::unlockProperties() {
        _properties.unlockAllProperties();
    }


}
