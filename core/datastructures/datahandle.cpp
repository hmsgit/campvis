// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#include "datahandle.h"

#include "tgt/assert.h"
#include "core/datastructures/abstractdata.h"

namespace campvis {
    DataHandle::DataHandle(AbstractData* data) 
        : _data(data)
        , _timestamp(clock())
    {
        init();
    }

    DataHandle::DataHandle(const DataHandle& rhs) 
        : _data(rhs._data)
        , _timestamp(rhs._timestamp)
    {
        init();
    }

    DataHandle& DataHandle::operator=(const DataHandle& rhs) {
        if (_data != rhs._data) {
            AbstractData* oldData = _data;
            _data = rhs._data;
            _timestamp = rhs._timestamp;
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

    void DataHandle::init() {
        if (_data == 0)
            return;

        if (! _data->isShareable()) {
            _data = _data->clone();
            _timestamp = clock();
        }
        _data->addReference();
    }

    clock_t DataHandle::getTimestamp() const {
        return _timestamp;
    }

}