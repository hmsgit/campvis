// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

#include "datacontainer.h"

#include "cgt/assert.h"
#include "cgt/logmanager.h"
#include "core/datastructures/abstractdata.h"

namespace campvis {
    const std::string DataContainer::loggerCat_ = "CAMPVis.core.datastructures.DataContainer";

    DataContainer::DataContainer(const std::string& name)
        : _name(name)
    {

    }

    DataContainer::~DataContainer() {
        _handles.clear();
    }

    DataHandle DataContainer::addData(const std::string& name, AbstractData* data) {
        if (name.empty()) {
            LERROR("Tried to add data with empty name to DataContainer.");
            return DataHandle(0);
        }

        cgtAssert(data != 0, "The Data must not be 0.");
        cgtAssert(!name.empty(), "The data's name must not be empty.");

        DataHandle dh(data);
        addDataHandle(name, dh);
        return dh;
    }

    void DataContainer::addDataHandle(const std::string& name, DataHandle dh) {
        if (name.empty()) {
            LERROR("Tried to add data with empty name to DataContainer.");
            return;
        }

        cgtAssert(dh.getData() != 0, "The data in the DataHandle must not be 0!");
        cgtAssert(!name.empty(), "The data's name must not be empty.");
        _handles.erase(name);
        _handles.insert(std::make_pair(name, dh));
 
        s_dataAdded.emitSignal(name, dh);
        s_changed.emitSignal();
    }

    bool DataContainer::hasData(const std::string& name) const {
        tbb::concurrent_hash_map<std::string, DataHandle>::const_accessor a;
        return _handles.find(a, name);
    }

    DataHandle DataContainer::getData(const std::string& name) const {
        tbb::concurrent_hash_map<std::string, DataHandle>::const_accessor a;
        if (_handles.find(a, name)) {
            return a->second;
        }
        else {
            return DataHandle(0);
        }
    }

    void DataContainer::removeData(const std::string& name) {
        _handles.erase(name);
    }

    std::vector< std::pair< std::string, DataHandle> > DataContainer::getDataHandlesCopy() const {
        std::vector< std::pair< std::string, DataHandle> > toReturn;
        toReturn.reserve(_handles.size());

        tbb::spin_mutex::scoped_lock lock(_localMutex);
        for (tbb::concurrent_hash_map<std::string, DataHandle>::const_iterator it = _handles.begin(); it != _handles.end(); ++it) {
            toReturn.push_back(std::make_pair(it->first, it->second));
        }

        return toReturn;
    }

    void DataContainer::clear() {
        _handles.clear();
    }

    const std::string& DataContainer::getName() const {
        return _name;
    }

    void DataContainer::setName(const std::string& name) {
        _name = name;
    }

}