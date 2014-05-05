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

#ifndef STRINGPROPERTY_H__
#define STRINGPROPERTY_H__

#include "core/properties/genericproperty.h"
#include <string>

namespace campvis {

    class CAMPVIS_CORE_API StringProperty : public GenericProperty<std::string> {
    public:
        /// Enumeration of the possible display types of this property
        enum DisplayType {
            BASIC_STRING,
            OPEN_FILENAME,
            SAVE_FILENAME,
            DIRECTORY
        };

        /**
         * Creates a new StringProperty
         * \param name      Property name (unchangable!)
         * \param title     Property title (e.g. used for GUI)
         * \param value     Initial value
         */
        StringProperty(const std::string& name, const std::string& title, const std::string& value = "", DisplayType displayType = BASIC_STRING);

        /**
         * Virtual Destructor
         **/
        virtual ~StringProperty();

        /**
         * Gets display type of this property.
         * \return _displayType
         **/
        DisplayType getDisplayType() const { return _displayType; }

        /**
         * Sets display type of this property.
         * \param _displayType New value for display type of this property.
         **/
        void setDisplayType(DisplayType val) { _displayType = val; }

        
    protected:
        DisplayType _displayType;

        static const std::string loggerCat_;
    };

}

#endif // STRINGPROPERTY_H__
