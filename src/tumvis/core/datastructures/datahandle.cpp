#include "datahandle.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "core/datastructures/abstractdata.h"

namespace TUMVis {
    const std::string DataHandle::loggerCat_ = "TUMVis.core.datastructures.DataHandle";


    DataHandle::DataHandle(AbstractData* data) 
        : _data(data)
    {
        init();
    }

    DataHandle::DataHandle(const DataHandle& rhs) 
        : _data(rhs._data)
    {
        init();
    }

    DataHandle& DataHandle::operator=(const DataHandle& rhs) {
        if (_data != rhs._data) {
            AbstractData* oldData = _data;
            _data = rhs._data;
            init();
            if (oldData) {
                oldData->removeReference();
            }
        }

        return *this;
    }

    DataHandle::~DataHandle() {
        if (_data)
            _data->removeReference();
    }


    const AbstractData* DataHandle::getData() const {
        return _data;
    }

    AbstractData* DataHandle::getData() {
        if (_data->isShared())
            _data = _data->clone();

        _data->markUnsharable();
        return _data;
    }

    void DataHandle::init() {
        if (_data == 0)
            return;

        if (! _data->isShareable())
            _data = _data->clone();
        _data->addReference();
    }

}