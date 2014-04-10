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

#ifndef datahandle_h__
#define datahandle_h__

#include <ctime>

#include "core/coreapi.h"

namespace campvis {
    class AbstractData;

    /**
     * A DataHandle is responsible to manage the lifetime of an AbstractData instance.
     * Therefore, it implements a reference counting technique in cooperation with AbstractData.
     * 
     * This class can be considered as thread safe under the following conditions:
     *  * A single DataHandle instance must not be accessed from different threads.
     *  * Concurrent access to the same AbstractData instance via different DataHandles is safe.
     * 
     * \note    For clarity: An AbstractData instance can be referenced by multiple DataHandles. As soon 
     *          as it is afterwards reference by 0 DataHandles, the AbstractData instance will be destroyed.
     *          Also remember that a DataHandle takes ownership of the given AbstractData instance. So do
     *          not delete it once it has been assigned to a DataHandle (respectively DataContainer) or mess
     *          with its reference counting!
     * \note    Reference counting implementation inspired from Scott Meyers: More Effective C++, Item 29
     */
    class CAMPVIS_CORE_API DataHandle {
    public:
        /**
         * Creates a new DataHandle for the given data.
         * \note    By passing the data to DataHandle you will transfer its ownership to the reference
         *          counting mechanism. Make sure not to interfere with it or delete \a data yourself!
         * \param   data    Data for the DataHandle
         */
        explicit DataHandle(AbstractData* data = 0);

        /**
         * Copy-constructor
         * \note    If \a rhs is not shareable, this implies a copy of the data!
         * \param   rhs     Source DataHandle
         */
        DataHandle(const DataHandle& rhs);

        /**
         * Assignment operator
         * \note    If \a rhs is not shareable, this implies a copy of the data!
         * \param   rhs     source DataHandle
         * \return  *this
         */
        DataHandle& operator=(const DataHandle& rhs);

        /**
         * Destructor, decrease the reference count of the managed AbstractData.
         */
        virtual ~DataHandle();

        /**
         * Grants const access to the managed AbstractData instance.
         * \return  _data;
         */
        const AbstractData* getData() const;

        /**
         * Gets the timestamp when this data has been created.
         * \return _timestamp
         */
        clock_t getTimestamp() const;


    private:
        /**
         * Initializes the reference counting for the data.
         */
        void init();

        AbstractData* _data;                    ///< managed data
        clock_t _timestamp;                     ///< Timestamp when this data has been created
    };

}

#endif // datahandle_h__