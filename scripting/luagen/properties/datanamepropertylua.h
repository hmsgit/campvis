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

#ifndef DATANAMEPROPERTYLUA_H__
#define DATANAMEPROPERTYLUA_H__

#include "core/properties/datanameproperty.h"
#include "scripting/scriptingapi.h"
#include "abstractpropertylua.h"
#include "propertyluafactory.h"

namespace campvis {
    /**
     * Lua generator for a DataNameProperty
     */
    class CAMPVIS_SCRIPTING_API DataNamePropertyLua : public AbstractPropertyLua {
    public:
        /**
         * Creates a new PropertyLua for the property \a property.
         * \param   property    The property the lua shall handle
         */
        DataNamePropertyLua(DataNameProperty* property);

        /**
         * Destructor
         */
        virtual ~DataNamePropertyLua();

        std::string getLuaScript(const std::string& propNamePrefix, const std::string& luaProc);
    };

    // explicitly instantiate template, so that it gets registered also over DLL boundaries.
    template class PropertyLuaRegistrar<DataNamePropertyLua, DataNameProperty, 10>;
}

#endif // DATANAMEPROPERTYLUA_H__
