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

#ifndef POSITIONDATA_H__
#define POSITIONDATA_H__

#include "cgt/vector.h"

#include "core/datastructures/abstractdata.h"

namespace campvis {

    /**
     * Data Container to store a position and (optional) rotation as quaternion
     */
    class CAMPVIS_CORE_API PositionData : public AbstractData {
    public:
        /**
         * Constructor, Creates a new light source.
         * \param   position       The initial position
         * \param   quaternion     The initial quaternion/orientation
         */
        explicit PositionData(const cgt::vec3 &position, const cgt::vec4 &quaternion = cgt::vec4(0.f, 0.f, 0.f, 0.f) );;

        /**
         * Virtual destructor
         */
        virtual ~PositionData();

        /// \see AbstractData::clone()
        virtual PositionData* clone() const;;

        /// \see AbstractData::getLocalMemoryFootprint()
        virtual size_t getLocalMemoryFootprint() const;;
        /// \see AbstractData::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;;
        /// \see AbstractData::getTypeAsString()
        virtual std::string getTypeAsString() const;

        /**
        * Gets the quaternion
        * \return _quaternion
        **/
        cgt::vec4 getQuaternion() const {
            return _quaternion;
        }

        /**
        * Sets the quaternion.
        * \param val New quaternion vector
        **/
        void setQuaternion(const cgt::vec4 &val) {
            _quaternion = val;
        }
        
        /**
        * Gets the position
        * \return _position
        **/
        cgt::vec3 getPosition() const {
            return _position;
        }

        /**
        * Sets the position.
        * \param val New position vector
        **/
        void setPosition(const cgt::vec3 &val) {
            _position = val;
        }
        

    protected:
        cgt::vec3 _position;                ///< the position
        cgt::vec4 _quaternion;              ///< The orientation quaternion
    };

}

#endif // POSITION_H__
