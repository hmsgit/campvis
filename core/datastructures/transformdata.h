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

#ifndef TRANSFORMDATA_H__
#define TRANSFORMDATA_H__

#include "cgt/vector.h"

#include "core/datastructures/abstractdata.h"

namespace campvis {

    /**
     * Data Container class for transforms. Stores a \a cgt::mat4 object.
     */
    class CAMPVIS_CORE_API TransformData : public AbstractData {
    public:
        /**
         * Constructor, Creates a new light source.
         * \param   transform       The transformation
         */
        explicit TransformData(const cgt::mat4 & transform);

        /**
         * Virtual destructor
         */
        virtual ~TransformData();

        /// \see AbstractData::clone()
        virtual TransformData* clone() const;
        /// \see AbstractData::getLocalMemoryFootprint()
        virtual size_t getLocalMemoryFootprint() const;
        /// \see AbstractData::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;
        /// \see AbstractData::getTypeAsString()
        virtual std::string getTypeAsString() const;

        /**
        * Gets the transformation
        * \return _transform
        **/
        cgt::mat4 getTransform() const {
            return _transform;
        }
        /**
        * Sets the transformation.
        * \param _transformation New transformation matrix
        **/
        void setTransform(const cgt::mat4 & val) {
            _transform = val;
        }
                

    protected:
        cgt::mat4 _transform;               ///< The transform
    };

}

#endif // TRANSFORMDATA_H__
