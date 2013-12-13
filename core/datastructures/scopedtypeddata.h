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

#ifndef SCOPEDTYPEDDATA_H__
#define SCOPEDTYPEDDATA_H__

#include "core/datastructures/datacontainer.h"

namespace campvis {

    /**
     * Proxy class for scoped strongly-typed access to the data of a DataContainer.
     * From the outside ScopedTypedData<T> behaves exactly like a const T*, but internally it preserves the
     * reference counting of a DataHandle. Use this class when you want temporary access to a strongly-typed
     * data item in a DataContainer but don't want to to the dynamic_cast yourself.
     *
     * \tparam  T   Base class of the DataHandle data to test for
     */
    template<typename T>
    struct ScopedTypedData {
        /**
         * Creates a new DataHandle to the data item with the key \a name in \a dc, that behaves like a T*.
         * \param   dc      DataContainer to grab data from
         * \param   name    Key of the DataHandle to search for
         */
        ScopedTypedData(const DataContainer& dc, const std::string& name)
            : dh(dc.getData(name))
            , data(0)
        {
            if (dh.getData() != 0) {
                data = dynamic_cast<const T*>(dh.getData());
                if (data == 0)
                    dh = DataHandle(0);
            }
        };

        /**
         * Implicit conversion operator to const T*.
         * \return  The data in the DataHandle, may be 0 when no DataHandle was found, or the data is of the wrong type.
         */
        operator const T*() {
            return data;
        }

        /**
         * Implicit arrow operator to const T*.
         * \return  The data in the DataHandle, may be 0 when no DataHandle was found, or the data is of the wrong type.
         */
        const T* operator->() const {
            return data;
        }

        /**
         * Gets the DataHandle.
         * \return dh
         */
        const DataHandle& getDataHandle() const {
            return dh;
        }

    private:
        /// Not copy-constructable
        ScopedTypedData(const ScopedTypedData& rhs);
        /// Not assignable
        ScopedTypedData& operator=(const ScopedTypedData& rhs);

        DataHandle dh;      ///< DataHandle
        const T* data;      ///< strongly-typed pointer to data, may be 0
    };

}

#endif // SCOPEDTYPEDDATA_H__