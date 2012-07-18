#include "datahandle.h"
#include "tgt/assert.h"
#include "tgt/logmanager.h"

namespace TUMVis {
    const std::string DataHandle::loggerCat_ = "TUMVis.core.datastructures.DataHandle";


    DataHandle::DataHandle(const DataContainer* owner, AbstractData* data) 
        : _data(data)
    {
        addOwner(this, owner);
    }

    DataHandle::~DataHandle() {
        delete _data;
    }

    void DataHandle::addOwner(const DataHandle* handle, const DataContainer* owner) {
        tgtAssert(handle != 0, "Handle must not be 0!");
        tgtAssert(owner != 0, "Owning DataContainer must not be 0!");
        tbb::spin_mutex::scoped_lock lock(handle->_localMutex);
        handle->_owners.insert(owner);
    }

    void DataHandle::removeOwner(const DataHandle* handle, const DataContainer* owner) {
        tgtAssert(handle != 0, "Handle must not be 0!");
        tgtAssert(owner != 0, "Owning DataContainer must not be 0!");
        {
            tbb::spin_mutex::scoped_lock lock(handle->_localMutex);
            handle->_owners.erase(owner);
        }
        if (handle->_owners.empty()) {
            delete handle;
        }
    }

    const AbstractData* DataHandle::getData() const {
        return _data;
    }

    AbstractData* DataHandle::getData() {
        return _data;
    }
}