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

#include "dataseries.h"

#include "cgt/assert.h"

namespace campvis {


    DataSeries::DataSeries()
        : AbstractData()
    {

    }

    DataSeries::~DataSeries() {
    }

    DataSeries* DataSeries::clone() const {
        DataSeries* toReturn = new DataSeries();
        for (size_t i = 0; i < _data.size(); ++i)
            toReturn->_data.push_back(_data[i]);

        return toReturn;
    }

    size_t DataSeries::getLocalMemoryFootprint() const {
        size_t toReturn = sizeof(DataHandle) * _data.capacity();
        for (size_t i = 0; i < _data.size(); ++i)
            toReturn += _data[i].getData()->getLocalMemoryFootprint();
        return toReturn;
    }

    size_t DataSeries::getVideoMemoryFootprint() const {
        size_t toReturn = 0;
        for (size_t i = 0; i < _data.size(); ++i)
            toReturn += _data[i].getData()->getVideoMemoryFootprint();
        return toReturn;
    }

    void DataSeries::addData(AbstractData* image) {
        _data.push_back(DataHandle(image));
    }

    void DataSeries::addData(DataHandle dh) {
        cgtAssert(dh.getData() != nullptr, "DataHandle must contain data!");
        _data.push_back(dh);
    }

    size_t DataSeries::getNumDatas() const {
        return _data.size();
    }

    DataHandle DataSeries::getData(size_t index) const {
        cgtAssert(index < _data.size(), "Index out of bounds.");
        return _data[index];
    }

    std::string DataSeries::getTypeAsString() const {
        return "Data Series";
    }


}