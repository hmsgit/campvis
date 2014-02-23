#include "luavmstate.h"

#include <iostream>
#include "globalluatable.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}


namespace campvis {

    LuaVmState::LuaVmState(bool loadStdLibs /*= true*/) : _luaState(0)
    {
        _luaState = luaL_newstate();

        // load standard Lua libraries
        if (loadStdLibs)
            luaL_openlibs(_luaState);
    }

    LuaVmState::~LuaVmState() {
        lua_close(_luaState);
    }

    void LuaVmState::logLuaError() {
        const char* errorMsg = lua_tostring(_luaState, -1);

        if (errorMsg == nullptr)
            std::cerr << "(error object is not a string)" << std::endl;
        else
            std::cerr << errorMsg << std::endl;

        lua_pop(_luaState, 1);
    }

    bool LuaVmState::execFile(const std::string& scriptPath) {
        // run a Lua script here; true is returned if there were errors
        if (luaL_dofile(_luaState, scriptPath.c_str())) {
            this->logLuaError();
            return false;
        }

        return true;
    }

    bool LuaVmState::execString(const std::string& scriptString) {
        if (luaL_dostring(_luaState, scriptString.c_str())) {
            this->logLuaError();
            return false;
        }

        return true;
    }

    std::shared_ptr<GlobalLuaTable> LuaVmState::getGlobalTable() {
        return std::shared_ptr<GlobalLuaTable>(new GlobalLuaTable(*this));
    }

    lua_State *LuaVmState::rawState() const {
        return _luaState;
    }

    void LuaVmState::callLuaFunc(int nargs, int nresults) {
        if (lua_pcall(_luaState, nargs, nresults, 0) != LUA_OK) {
            this->logLuaError();
        }
    }
}
