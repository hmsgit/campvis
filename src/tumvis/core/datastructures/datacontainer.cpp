#include "datacontainer.h"

namespace TUMVis {
    const std::string DataContainer::loggerCat_ = "TUMVis.core.datastructures.DataContainer";

    DataContainer::DataContainer() {

    }

    DataContainer::~DataContainer() {
        // remove ownership op all owned DataHandles
        for (std::map<std::string, const DataHandle*>::iterator it = _handles.begin(); it != _handles.end(); ++it) {
            DataHandle::removeOwner(it->second, this);
        }
        _handles.clear();
    }

    const DataHandle* DataContainer::addData(const std::string& name, AbstractData* data) {
        DataHandle* dh = new DataHandle(this, data);
        _handles.insert(std::make_pair(name, dh));
        return dh;
    }

    void DataContainer::addDataHandle(const std::string& name, const DataHandle* dh) {
        _handles.insert(std::make_pair(name, dh));
        DataHandle::addOwner(dh, this);
    }

    bool DataContainer::hasData(const std::string& name) const {
        return (_handles.find(name) != _handles.end());
    }

    const DataHandle* DataContainer::getData(const std::string& name) const {
        std::map<std::string, const DataHandle*>::const_iterator it = _handles.find(name);
        if (it == _handles.end())
            return 0;
        else
            return it->second;
    }
}