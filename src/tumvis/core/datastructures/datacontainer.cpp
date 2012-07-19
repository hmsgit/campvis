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
        DataHandle* dh = new DataHandle(data);
        addDataHandle(name, dh);
        return dh;
    }

    void DataContainer::addDataHandle(const std::string& name, const DataHandle* dh) {
        {
            tbb::spin_mutex::scoped_lock lock(_localMutex);
            std::map<std::string, const DataHandle*>::iterator it = _handles.lower_bound(name);
            if (it != _handles.end() && it->first == name) {
                DataHandle::removeOwner(it->second, this);
                it->second = dh;
            }
            else {
                _handles.insert(it, std::make_pair(name, dh));
            }
        }
        DataHandle::addOwner(dh, this);
        s_dataAdded(name, dh);
    }

    bool DataContainer::hasData(const std::string& name) const {
        tbb::spin_mutex::scoped_lock lock(_localMutex);
        return (_handles.find(name) != _handles.end());
    }

    const DataHandle* DataContainer::getData(const std::string& name) const {
        tbb::spin_mutex::scoped_lock lock(_localMutex);
        std::map<std::string, const DataHandle*>::const_iterator it = _handles.find(name);
        if (it == _handles.end())
            return 0;
        else
            return it->second;
    }
}