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

#ifndef BUTTONPROPERTYLUA_H__
#define BUTTONPROPERTYLUA_H__

#include "abstractpropertylua.h"
#include "propertyluafactory.h"
#include "core/properties/buttonproperty.h"

class QPushButton;

namespace campvis {
    /**
     * Lua for a Camera.
     * For now just offering read-access.
     */
    class ButtonPropertyLua : public AbstractPropertyLua {
    public:
        /**
         * Creates a new ButtonPropertyLua for the property \a property.
         * \param   property    The property the lua shall handle
         * \param   parent      Parent Qt lua
         */
        ButtonPropertyLua(ButtonProperty* property, DataContainer* dataContainer = nullptr);

        /**
         * Destructor
         */
        virtual ~ButtonPropertyLua();

        std::string getLuaScript();
    };

    // explicitly instantiate template, so that it gets registered also over DLL boundaries.
    //template class PropertyLuaRegistrar<ButtonPropertyLua, ButtonProperty>;
}

#endif // BUTTONPROPERTYLUA_H__
