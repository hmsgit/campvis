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

#ifndef COLORPROPERTYLUA_H__
#define COLORPROPERTYLUA_H__

#include "abstractpropertylua.h"
#include "propertyluafactory.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/colorproperty.h"
#include "scripting/scriptingapi.h"


namespace campvis {
    /**
     * Lua generator for a ColorProperty
     */
    class CAMPVIS_SCRIPTING_API ColorPropertyLua : public AbstractPropertyLua {
    public:
        /**
         * Creates a new FloatPropertyLua for the property \a property.
         * \param   property    The property the lua shall handle
         */
        ColorPropertyLua(ColorProperty* property);

        /**
         * Destructor
         */
        virtual ~ColorPropertyLua();

        std::string getLuaScript(const std::string& propNamePrefix, const std::string& luaProc);
    }; 

// ================================================================================================

    // explicitly instantiate template, so that it gets registered also over DLL boundaries.   
    template class PropertyLuaRegistrar<ColorPropertyLua, ColorProperty>;

}

#endif // COLORPROPERTYLUA_H__
