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

#include "regularluatable.h"

// Add property luas to ensure build
// TODO: find a better way
#include "scripting/luagen/properties/boolpropertylua.h"
#include "scripting/luagen/properties/floatpropertylua.h"
#include "scripting/luagen/properties/intpropertylua.h"
#include "scripting/luagen/properties/optionpropertylua.h"
#include "scripting/luagen/properties/stringpropertylua.h"
#include "scripting/luagen/properties/colorpropertylua.h"
#include "scripting/luagen/properties/datanamepropertylua.h"
#include "scripting/luagen/properties/metapropertylua.h"
#include "scripting/luagen/properties/propertycollectionluascriptgenerator.h"
#include "scripting/luagen/properties/transferfunctionpropertylua.h"

namespace campvis {

    RegularLuaTable::RegularLuaTable(std::shared_ptr<LuaTable> parent, std::string name)
        : LuaTable(parent->_luaVmState)
        , _parent(parent)
        , _name(name)
    {}

    RegularLuaTable::~RegularLuaTable() {}

    bool RegularLuaTable::isValid() {
        bool result = false;
        _parent->pushField(_name);

        {
            LuaStateMutexType::scoped_lock lock(_luaVmState.getMutex());

            if (lua_istable(_luaVmState.rawState(), -1) == 1)
                result = true;

            // Pop the table
            lua_pop(_luaVmState.rawState(), 1);
        }

        return result;
    }

    std::shared_ptr<LuaTable> RegularLuaTable::getTable(const std::string& name) {
        return std::shared_ptr<LuaTable>(new RegularLuaTable(this->shared_from_this(), name));
    }

    void RegularLuaTable::callInstanceMethod(const std::string& name) {
        _parent->pushField(_name);

        {
            LuaStateMutexType::scoped_lock lock(_luaVmState.getMutex());
            lua_getfield(_luaVmState.rawState(), -1, name.c_str());
        }

        _parent->pushField(_name);
        _luaVmState.callLuaFunc(1, 0);

        {
            LuaStateMutexType::scoped_lock lock(_luaVmState.getMutex());
            // Pop the table
            lua_pop(_luaVmState.rawState(), 1);
        }
    }

    void RegularLuaTable::pushField(const std::string& name) {
        _parent->pushField(_name);

        {
            LuaStateMutexType::scoped_lock lock(_luaVmState.getMutex());
            lua_getfield(_luaVmState.rawState(), -1, name.c_str());
        }
    }
}
