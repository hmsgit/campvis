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

#include "luavmstate.h"

#include <iostream>
#include "globalluatable.h"

#include "cgt/logmanager.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

static int lua_campvis_print(lua_State* L) {
    int nargs = lua_gettop(L);
    lua_getglobal(L, "tostring");
    std::string str;

    for (int i=1; i <= nargs; i++) {
        const char *s;
        size_t l;
        lua_pushvalue(L, -1);  /* function to be called */
        lua_pushvalue(L, i);   /* value to print */
        lua_call(L, 1, 1);
        s = lua_tolstring(L, -1, &l);  /* get result */
        if (s == NULL)
            return luaL_error(L, LUA_QL("tostring") " must return a string to " LUA_QL("print"));

        if (i>1) 
            str += "\t";
        str += s;
        lua_pop(L, 1);  /* pop result */
    }

    LogMgr.log("Lua", cgt::LuaInfo, str);
    return 0;
}

static int lua_campvis_debug(lua_State* L) {
    for (;;) {
        char buffer[250];
        //luai_writestringerror("%s", "lua_debug> ");
        if (fgets(buffer, sizeof(buffer), stdin) == 0 || strcmp(buffer, "cont\n") == 0)
            return 0;
        if (luaL_loadbuffer(L, buffer, strlen(buffer), "=(debug command)") || lua_pcall(L, 0, 0, 0))
            LDEBUGC("Lua", lua_tostring(L, -1));
        lua_settop(L, 0);  /* remove eventual returns */
    }
}


namespace campvis {

    LuaVmState::LuaVmState(bool loadStdLibs /*= true*/)
        : _luaState(0)
        , _luaStateMutex()
    {
        _luaState = luaL_newstate();

        // load standard Lua libraries
        if (loadStdLibs)
            luaL_openlibs(_luaState);

        /*
         * Store a pointer to the mutex guarding access to _luaState in the state's registry; this
         * way code that accesses Lua state directly (e.g. connections between sigslot's signals and
         * slots defined in Lua) has access to it and can lock it when necessary.
         */
        lua_pushlightuserdata(_luaState, static_cast<void*>(_luaState));
        lua_pushlightuserdata(_luaState, static_cast<void*>(&_luaStateMutex));
        lua_settable(_luaState, LUA_REGISTRYINDEX);
    }

    LuaVmState::~LuaVmState() {
        LuaStateMutexType::scoped_lock lock(_luaStateMutex);
        lua_close(_luaState);
    }

    void LuaVmState::logLuaError() {
        const char* errorMsg = lua_tostring(_luaState, -1);

        if (errorMsg == nullptr)
            LogMgr.log("Lua", cgt::LuaError, "(error object is not a string)");
        else
            LogMgr.log("Lua", cgt::LuaError, errorMsg);

        lua_pop(_luaState, 1);
    }

    bool LuaVmState::execFile(const std::string& scriptPath) {
        LuaStateMutexType::scoped_lock lock(_luaStateMutex);

        // run a Lua script here; true is returned if there were errors
        if (luaL_dofile(_luaState, scriptPath.c_str())) {
            this->logLuaError();
            return false;
        }

        return true;
    }

    bool LuaVmState::execString(const std::string& scriptString) {
        LuaStateMutexType::scoped_lock lock(_luaStateMutex);

        if (luaL_dostring(_luaState, scriptString.c_str())) {
            this->logLuaError();
            return false;
        }

        return true;
    }

    std::shared_ptr<GlobalLuaTable> LuaVmState::getGlobalTable() {
        if (! _globalLuaTable)
            _globalLuaTable = std::make_shared<GlobalLuaTable>(*this);

        return _globalLuaTable;
    }

    lua_State* LuaVmState::rawState() const {
        return _luaState;
    }

    LuaStateMutexType& LuaVmState::getMutex() {
        return _luaStateMutex;
    }

    void LuaVmState::callLuaFunc(int nargs, int nresults) {
        LuaStateMutexType::scoped_lock lock(_luaStateMutex);

        if (lua_pcall(_luaState, nargs, nresults, 0) != LUA_OK) {
            this->logLuaError();
        }
    }

    void LuaVmState::redirectLuaPrint() {
        LuaStateMutexType::scoped_lock lock(_luaStateMutex);

        static const struct luaL_Reg printlib[] = {
            {"print", &lua_campvis_print},
            {"debug", &lua_campvis_debug},
            {NULL, NULL} /* end of array */
        };

        lua_getglobal(_luaState, "_G");
        luaL_register(_luaState, NULL, printlib);
        lua_pop(_luaState, 1);
    }

}
