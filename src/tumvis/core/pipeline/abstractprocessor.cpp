#include "abstractprocessor.h"

namespace TUMVis {

    const std::string AbstractProcessor::loggerCat_ = "TUMVis.core.datastructures.Processor";


    AbstractProcessor::AbstractProcessor() {

    }

    AbstractProcessor::~AbstractProcessor() {

    }

    void AbstractProcessor::addDataHandle(const std::string& name, const DataHandle* dh) {
        _data.addDataHandle(name, dh);
    }

    const DataContainer& AbstractProcessor::getDataContainer() const {
        return _data;
    }

    int AbstractProcessor::getInvalidationLevel() const {
        return _invalidationLevel;
    }

    void AbstractProcessor::setInvalidationLevel(InvalidationLevel il) {
        if (il == VALID) {
            _invalidationLevel = static_cast<int>(VALID);
        }
        else {
            _invalidationLevel |= static_cast<int>(il);
        }        
    }
}
