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

#ifndef DATANAMEPROPERTYLUA_H__
#define DATANAMEPROPERTYLUA_H__

#include "core/properties/datanameproperty.h"
#include "abstractpropertylua.h"
#include "propertyluafactory.h"

class QComboBox;
class QLineEdit;

namespace campvis {
    class DataNameProperty;
    class DataContainer;
    class DataHandle;

    /**
     * Lua for a DataNameProperty
     */
    class DataNamePropertyLua : public AbstractPropertyLua {
    public:
        /**
         * Creates a new PropertyLua for the property \a property.
         * \param   property    The property the lua shall handle
         * \param   dc          DataContainer this property uses to populate the combo box (may be 0).
         * \param   parent      Parent Qt lua
         */
        DataNamePropertyLua(DataNameProperty* property, DataContainer* dc);

        /**
         * Destructor
         */
        virtual ~DataNamePropertyLua();

        std::string getLuaScript();
    };

    // explicitly instantiate template, so that it gets registered also over DLL boundaries.
    template class PropertyLuaRegistrar<DataNamePropertyLua, DataNameProperty, 10>;
}

#endif // DATANAMEPROPERTYLUA_H__
