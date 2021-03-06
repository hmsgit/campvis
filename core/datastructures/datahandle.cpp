// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
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

#include "cgt/assert.h"
#include "core/datastructures/abstractdata.h"

namespace campvis {
    DataHandle::DataHandle(AbstractData* data) 
        : _timestamp(clock())
    {
        if (data) {
            if (data->_weakPtr.expired()) {
                _ptr = std::shared_ptr<AbstractData>(data);
                data->_weakPtr = _ptr;
            }
            else {
                _ptr = std::shared_ptr<AbstractData>(data->_weakPtr);
            }
        }
    }

    DataHandle::DataHandle(const DataHandle& rhs) 
        : _ptr(rhs._ptr)
        , _timestamp(rhs._timestamp)
    {

    }

    DataHandle& DataHandle::operator=(const DataHandle& rhs) {
        if (this != &rhs) {
            _ptr = rhs._ptr;
            _timestamp = rhs._timestamp;
        }

        return *this;
    }

    DataHandle::~DataHandle() {

    }


    const AbstractData* DataHandle::getData() const {
        return _ptr.get();
    }

    clock_t DataHandle::getTimestamp() const {
        return _timestamp;
    }

}