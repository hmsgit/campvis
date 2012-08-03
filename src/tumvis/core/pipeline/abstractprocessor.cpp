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
            s_invalidated(this);
        }
    }

    void AbstractProcessor::init() {
    }

    void AbstractProcessor::deinit() {
    }


    void AbstractProcessor::lockProperties() {
        lockAllProperties();
    }

    void AbstractProcessor::unlockProperties() {
        unlockAllProperties();
    }

    void AbstractProcessor::onPropertyChanged(const AbstractProperty* prop) {
        applyInvalidationLevel(prop->getInvalidationLevel());
    }

}
