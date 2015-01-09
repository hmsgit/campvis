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

#ifndef STATUSPROPERTY_H__
#define STATUSPROPERTY_H__

#include "core/properties/genericproperty.h"
#include <string>
#include <tuple>

namespace campvis {

    // simple class to store status as a combination of string and type
    struct Status {
    public:
        /// Enumeration of the possible status types
        enum StatusType {
            UNKNOWN,
            OK,
            WARNING,
            CRITICAL
        };


        Status(std::string string_arg, StatusType status_arg)
            : string(string_arg)
            , status(status_arg) 
        {};


        bool operator==(const Status & rhs) {
            return status == rhs.status && string == rhs.string;
        };

        bool operator!=(const Status &rhs) {
            return status != rhs.status || string != rhs.string;
        };
   
        std::string string;
        StatusType status;
    };

    class CAMPVIS_CORE_API StatusProperty : public GenericProperty<Status> {
    public:


        /**
         * Creates a new StatusProperty
         * \param name      Property name (unchangable!)
         * \param title     Property title (e.g. used for GUI)
         * \param value     Initial value
         */
        StatusProperty(const std::string& name, const std::string& title, const Status & value = Status(std::string(""), Status::UNKNOWN));

        /**
         * Virtual Destructor
         **/
        virtual ~StatusProperty();

        // convenience setter
        void setStatus(std::string str, Status::StatusType status)
        {
            setValue(Status(str, status));
        };

        
    protected:

        static const std::string loggerCat_;
    };

}

#endif // STATUSPROPERTY_H__
