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

#include "positiondata.h"

namespace campvis {

    PositionData::PositionData( const cgt::vec3 &position, const cgt::vec4 &quaternion /*= cgt::vec4(0.f, 0.f, 0.f, 0.f) */ )
        : AbstractData()
        , _position(position)
        , _quaternion(quaternion)
    {

    }

    PositionData::~PositionData() {

    }

    PositionData* PositionData::clone() const {
        return new PositionData(*this);
    }

    size_t PositionData::getLocalMemoryFootprint() const {
        return sizeof(PositionData);
    }

    size_t PositionData::getVideoMemoryFootprint() const {
        return 0;
    }

    std::string PositionData::getTypeAsString() const {
        return "Position Data";
    }

}
