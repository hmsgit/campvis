#include "abstractprocessor.h"

namespace TUMVis {

    const std::string AbstractProcessor::loggerCat_ = "TUMVis.core.datastructures.Processor";


    AbstractProcessor::AbstractProcessor() {

    }

    AbstractProcessor::~AbstractProcessor() {

    }

    void AbstractProcessor::addDataHandle(const std::string& name, const DataHandle* dh) {
        _dataContainer.addDataHandle(name, dh);
    }

    const DataContainer& AbstractProcessor::getDataContainer() const {
        return _dataContainer;
    }

    const InvalidationLevel& AbstractProcessor::getInvalidationLevel() const {
        return _invalidationLevel;
    }

    void AbstractProcessor::applyInvalidationLevel(InvalidationLevel::NamedLevels nl) {
        _invalidationLevel.setLevel(nl);
    }

    void AbstractProcessor::onNotify(const PropertyObserverArgs& poa) {
        _invalidationLevel.setLevel(poa._invalidationLevel);

        // If processor is no longer valid, notify observers
        if (! _invalidationLevel.isValid()) {
            notifyObservers(ProcessorObserverArgs(this, _invalidationLevel));
        }
    }

    PropertyCollection& AbstractProcessor::getPropertyCollection() {
        return _properties;
    }
}
