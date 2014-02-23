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
        lua_getglobal(_luaVmState.rawState(), name.c_str());
    }
}
