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

#ifndef DATANAMEPROPERTY_H__
#define DATANAMEPROPERTY_H__

#include "core/properties/genericproperty.h"

#include <ctime>
#include <set>
#include <string>

namespace campvis {

    /**
     * Specialization of a StringProperty to specify the keys (string IDs) for DataHandles to read/write.
     * Offers an additional DataAccessInfo flag whether processor will read or write from/to that ID.
     */
    class DataNameProperty : public GenericProperty<std::string> {
    public:
        /**
         * DataAccessInfo flag defining whether processor will read or write from/to the corresponding DataHandle.
         */
        enum DataAccessInfo {
            READ,
            WRITE
        };

        /**
         * Creates a new DataNameProperty
         * \param name      Property name (unchangable!)
         * \param title     Property title (e.g. used for GUI)
         * \param value     Initial value
         * \param access    DataAccessInfo flag defining whether processor will read or write from/to the DataHandle with the given ID.
         * \param invalidationLevel  Invalidation level that this property triggers
         */
        DataNameProperty(const std::string& name, const std::string& title, const std::string& value, DataAccessInfo access, int invalidationLevel = AbstractProcessor::INVALID_RESULT);

        /**
         * Virtual Destructor
         **/
        virtual ~DataNameProperty();


        /**
         * Returns the access info of this property defining whether component will read or write.
         * \return  _accessInfo
         */
        DataAccessInfo getAccessInfo() const;

    protected:

        DataAccessInfo _accessInfo;

        static const std::string loggerCat_;
    };

}

#endif // DATANAMEPROPERTY_H__
