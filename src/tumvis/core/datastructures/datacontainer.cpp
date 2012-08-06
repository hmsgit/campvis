#include "datacontainer.h"

#include "tgt/assert.h"
#include "core/datastructures/abstractdata.h"

namespace TUMVis {
    const std::string DataContainer::loggerCat_ = "TUMVis.core.datastructures.DataContainer";

    DataContainer::DataContainer() {

    }

    DataContainer::~DataContainer() {
        // remove ownership op all owned DataHandles
        for (std::map<std::string, const DataHandle*>::iterator it = _handles.begin(); it != _handles.end(); ++it) {
            delete it->second;
        }
        _handles.clear();
    }

    const DataHandle* DataContainer::addData(const std::string& name, AbstractData* data) {
        DataHandle* dh = new DataHandle(data);
        addDataHandle(name, dh);
        return dh;
    }

    void DataContainer::addDataHandle(const std::string& name, const DataHandle* dh) {
        tgtAssert(dh != 0, "DataHandle must not be 0.");
        {
            tbb::spin_mutex::scoped_lock lock(_localMutex);
            std::map<std::string, const DataHandle*>::iterator it = _handles.lower_bound(name);
            if (it != _handles.end() && it->first == name) {
                delete it->second;
                it->second = dh;
            }
            else {
                _handles.insert(it, std::make_pair(name, dh));
            }
        }
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
            return new DataHandle(*it->second);
    }

    void DataContainer::removeData(const std::string& name) {
        tbb::spin_mutex::scoped_lock lock(_localMutex);
        std::map<std::string, const DataHandle*>::const_iterator it = _handles.find(name);
        if (it != _handles.end()) {
            delete it->second;
            _handles.erase(it);
        }
    }
}