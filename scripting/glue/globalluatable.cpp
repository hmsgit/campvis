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

#include "globalluatable.h"

#include "regularluatable.h"


namespace campvis {

    GlobalLuaTable::GlobalLuaTable(LuaVmState& luaVmState)
        : LuaTable(luaVmState)
    {}

    GlobalLuaTable::~GlobalLuaTable() {}

    bool GlobalLuaTable::isValid() {
        // Each Lua VM has a global table
        return true;
    }

    std::shared_ptr<LuaTable> GlobalLuaTable::getTable(const std::string& name) {
        return std::shared_ptr<LuaTable>(new RegularLuaTable(this->shared_from_this(), name));
    }

    void GlobalLuaTable::callInstanceMethod(const std::string& name) {
        this->pushField(name);
        _luaVmState.callLuaFunc(1, 0);
    }

    void GlobalLuaTable::pushField(const std::string& name) {
        LuaStateMutexType::scoped_lock lock(_luaVmState.getMutex());
        lua_getglobal(_luaVmState.rawState(), name.c_str());
    }
}
