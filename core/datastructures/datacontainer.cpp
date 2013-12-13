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

#include "datacontainer.h"

#include "tgt/assert.h"
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
        tgtAssert(data != 0, "The Data must not be 0.");
        DataHandle dh(data);
        addDataHandle(name, dh);
        return dh;
    }

    void DataContainer::addDataHandle(const std::string& name, const DataHandle& dh) {
        tgtAssert(dh.getData() != 0, "The data in the DataHandle must not be 0!");
        {
            //tbb::spin_mutex::scoped_lock lock(_localMutex);
            tbb::concurrent_unordered_map<std::string, DataHandle>::iterator it = _handles.find(name);
            if (it != _handles.end()/* && it->first == name*/) {
                it->second = dh;
            }
            else {
                _handles.insert(/*it,*/ std::make_pair(name, dh));
            }
        }
        s_dataAdded(name, dh);
        s_changed();
    }

    bool DataContainer::hasData(const std::string& name) const {
        //tbb::spin_mutex::scoped_lock lock(_localMutex);
        return (_handles.find(name) != _handles.end());
    }

    DataHandle DataContainer::getData(const std::string& name) const {
        //tbb::spin_mutex::scoped_lock lock(_localMutex);
        tbb::concurrent_unordered_map<std::string, DataHandle>::const_iterator it = _handles.find(name);
        if (it == _handles.end())
            return DataHandle(0);
        else
            return it->second;
    }

    void DataContainer::removeData(const std::string& name) {
        tbb::spin_mutex::scoped_lock lock(_localMutex);
        tbb::concurrent_unordered_map<std::string, DataHandle>::const_iterator it = _handles.find(name);
        if (it != _handles.end()) {
            _handles.unsafe_erase(it);
        }
    }

    std::vector< std::pair< std::string, DataHandle> > DataContainer::getDataHandlesCopy() const {
        std::vector< std::pair< std::string, DataHandle> > toReturn;
        toReturn.reserve(_handles.size());

        tbb::spin_mutex::scoped_lock lock(_localMutex);
        for (tbb::concurrent_unordered_map<std::string, DataHandle>::const_iterator it = _handles.begin(); it != _handles.end(); ++it) {
            toReturn.push_back(std::make_pair(it->first, it->second));
        }

        return toReturn;
    }

//     tbb::concurrent_unordered_map<std::string, DataHandle> DataContainer::getHandlesCopy() const {
//         //tbb::spin_mutex::scoped_lock lock(_localMutex);
//         return _handles;
//     }

    void DataContainer::clear() {
        //tbb::spin_mutex::scoped_lock lock(_localMutex);
        _handles.clear();
    }

    const std::string& DataContainer::getName() const {
        return _name;
    }

    void DataContainer::setName(const std::string& name) {
        _name = name;
    }

}