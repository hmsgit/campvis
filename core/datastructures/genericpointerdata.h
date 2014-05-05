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

#ifndef GENERICPOINTERDATA_H__
#define GENERICPOINTERDATA_H__


#include "core/datastructures/abstractdata.h"
#include <string>

namespace campvis {

    /**
     * Class that generically wraps around a pointer of the template type and takes ownership of it.
     * \tparam  T   Type of the pointer this AbstractData wraps around.
     */
    template<typename T>
    class GenericPointerData : public AbstractData {
    public:
        /**
         * Creates a new GenericPointerData and initializes its pointer with \a data.
         * \param   data    The initial pointer for this data, may be 0, GenericPointerData takes ownerwhip.
         */
        explicit GenericPointerData(T* data)
            : AbstractData()
            , _data(data)
        {};

        /**
         * Destructor, deletes the pointer.
         */
        virtual ~GenericPointerData() {
            delete _data;
        };

        /**
         * Returns the pointer to the wrapped data.
         * \return  _data, may be 0, GenericPointerData has ownership
         */
        const T* getData() const {
            return _data;
        };

        /**
         * Returns the pointer to the wrapped data.
         * \return  _data, may be 0, GenericPointerData has ownership
         */
        T* getData() {
            return _data;
        };

        /**
         * Sets the data to \a data.
         * \param   data    The new pointer for this data, may be 0, GenericPointerData takes ownerwhip.
         */
        void setData(T* data) {
            _data = data;
        };


        /**
         * Prototype - clone method, some people call this virtual constructor...
         * \return  A SHALLOW copy of this object.
         */
        virtual AbstractData* clone() const {
            // FIXME: This is only a shallow copy - not what you expect from clone!
            return new GenericPointerData<T>(_data);
        };

        /**
         * Returns the local memory footprint of the data in bytes.
         * \return  Number of bytes occupied in local memory by the data.
         */
        virtual size_t getLocalMemoryFootprint() const {
            return sizeof(T) + sizeof(T*);
        };

        /**
         * Returns the video memory footprint of the data in bytes.
         * \return  Number of bytes occupied in video memory by the data.
         */
        virtual size_t getVideoMemoryFootprint() const {
            return 0;
        };

    protected:
        T* _data;           ///< Pointer to the data.
    };
}

#endif // GENERICPOINTERDATA_H__
