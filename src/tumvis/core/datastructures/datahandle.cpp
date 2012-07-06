#include "datahandle.h"

namespace TUMVis {
    const std::string DataHandle::loggerCat_ = "TUMVis.core.datastructures.DataHandle";


    DataHandle::DataHandle(const DataContainer* owner, AbstractData* data) 
        : _data(data)
    {
        addOwner(this, owner);
    }

    DataHandle::DataHandle(const DataHandle& /*rhs*/) {
        // IMHO a DataHandle does not need a copy-constructor - in particular it could be a bad idea to use
        // one, because it does not exactly what you expect. If you really need a copy-constructor, please
        // make sure to implement it correctly. From my current point of view, you at least need to clone
        // _data but then you still don't know anything about any owners of the new DataHandle.
        LERROR("Do you really want to use the copy-constructor? If so, please implement it yourself, but beware: This might be dangerous...");
    }

    DataHandle& DataHandle::operator=(const DataHandle& /*rhs*/) {
        // IMHO a DataHandle does not need an assignment-operator - in particular it could be a bad idea to use
        // one, because it does not exactly what you expect. If you really need an assignment-operator, please
        // make sure to implement it correctly.
        LERROR("Do you really want to use the assignment-operator? If so, please implement it yourself, but beware: This might be dangerous...");
        return *this; //< of course this is evil - just want to make the compiler happy...
    }

    DataHandle::~DataHandle() {
        delete _data;
    }

    void DataHandle::addOwner(const DataHandle* handle, const DataContainer* owner) {
        handle->_owners.insert(owner);
    }

    void DataHandle::removeOwner(const DataHandle* handle, const DataContainer* owner) {
        handle->_owners.erase(owner);
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