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
}
