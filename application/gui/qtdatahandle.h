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

#ifndef QTQtDataHandle_H__
#define QTQtDataHandle_H__

#include "core/datastructures/datahandle.h"
#include <QMetaType>

namespace campvis {

    /**
     * Wraps around a DataHandle to make it suitable for QVariant and queued connections.
     */
    class QtDataHandle : public DataHandle {
    public:
        /**
         * Creates a new QtDataHandle for the given data.
         * \note    By passing the data to QtDataHandle you will transfer its ownership to the reference
         *          counting mechanism. Make sure not to interfere with it or delete \a data yourself!
         * \param   data    Data for the QtDataHandle
         */
        explicit QtDataHandle(AbstractData* data = 0)
            : DataHandle(data)
        {}

        /**
         * Copy-constructor
         * \note    If \a rhs is not shareable, this implies a copy of the data!
         * \param   rhs     Source QtDataHandle
         */
        explicit QtDataHandle(const QtDataHandle& rhs)
            : DataHandle(rhs)
        {}

        /**
         * Copy-constructor
         * \note    If \a rhs is not shareable, this implies a copy of the data!
         * \param   rhs     Source DataHandle
         */
        QtDataHandle(const DataHandle& rhs)
            : DataHandle(rhs)
        {}

        /**
         * Assignment operator
         * \note    If \a rhs is not shareable, this implies a copy of the data!
         * \param   rhs     source QtDataHandle
         * \return  *this
         */
        QtDataHandle& operator=(const QtDataHandle& rhs) {
            DataHandle::operator=(rhs);
            return *this;
        }

        /**
         * Destructor, decrease the reference count of the managed AbstractData.
         */
        virtual ~QtDataHandle() {
        }
    };
}

Q_DECLARE_METATYPE(campvis::QtDataHandle);

#endif // QTQtDataHandle_H__
