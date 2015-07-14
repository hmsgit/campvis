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
#include "luavmstate.h"

#include "cgt/assert.h"
#include "cgt/logmanager.h"
#include "core/tools/stringutils.h"


namespace campvis {

    RegularLuaTable::RegularLuaTable(std::shared_ptr<LuaTable> parent, std::string name)
        : LuaTable(parent->_luaVmState)
        , _parent(parent)
        , _name(name)
    {
    }

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

    void RegularLuaTable::callInstanceMethod(const std::string& name) {
        LuaStateMutexType::scoped_lock lock(_luaVmState.getMutex());
        _parent->pushField(_name);
        
        // FIXME: not sure whether this really works for deeply nested instance methods, the second pushField() call looks suspicious!
        lua_getfield(_luaVmState.rawState(), -1, name.c_str());
        _parent->pushField(_name);
        _luaVmState.callLuaFunc(1, 0); // probably we want a "lua_getfield(_luaVmState.rawState(), -2, name.c_str())" here

        _parent->popRecursive();
    }
    
    void RegularLuaTable::pushField(const std::string& name) {
        auto it = _valueMap.find(name);
        if (it != _valueMap.end()) {
            LuaStateMutexType::scoped_lock lock(_luaVmState.getMutex());
            lua_State* L = _luaVmState.rawState();
            _parent->pushField(_name);

            if (it->second.keyIsNumber)
                lua_pushnumber(L, StringUtils::fromString<double>(name));
            else
                lua_pushstring(L, name.c_str());
            lua_gettable(L, -2);
        }
        else {
            cgtAssert(false, "Field not in valueMap, this should not happen.");
        }
    }
    
    void RegularLuaTable::popRecursive() {
        LuaStateMutexType::scoped_lock lock(_luaVmState.getMutex());
        lua_pop(_luaVmState.rawState(), 1);
        _parent->popRecursive();
    }

    void RegularLuaTable::populateValueMap() {
        _valueMap.clear();
        LuaStateMutexType::scoped_lock lock(_luaVmState.getMutex());
        lua_State* L = _luaVmState.rawState();

        // load this table's name onto stack
        _parent->pushField(_name);
        iterateOverTableAndPopulateValueMap(L);
        _parent->popRecursive();
    }

    LuaTable* RegularLuaTable::getParentTable() {
        return _parent.get();
    }

}
