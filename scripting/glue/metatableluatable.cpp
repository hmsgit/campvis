#include "luavmstate.h"

#include "cgt/assert.h"
#include "cgt/logmanager.h"
#include "core/tools/stringutils.h"

namespace campvis {

    MetatableLuaTable::MetatableLuaTable(std::shared_ptr<LuaTable> parent, std::string fieldName)
        : LuaTable(parent->_luaVmState)
        , _parent(parent)
        , _fieldName(fieldName)
    {
    }

    MetatableLuaTable::~MetatableLuaTable() {}

    bool MetatableLuaTable::isValid() {
        bool toReturn = false;
        LuaStateMutexType::scoped_lock lock(_luaVmState.getMutex());
        lua_State* L = _luaVmState.rawState();

        _parent->pushField(_fieldName);
        if (!lua_getmetatable(_luaVmState.rawState(), -1)) {
            lua_pop(L, 1);
            toReturn = true;
        }
        _parent->popRecursive();

        return toReturn;
    }

    void MetatableLuaTable::callInstanceMethod(const std::string& name) {
        LuaStateMutexType::scoped_lock lock(_luaVmState.getMutex());

        cgtAssert(false, "Method not verified, use with care!");

        // recursively push this metatable onto stack, so that we can refer to it later
        _parent->pushField(_fieldName);

        // push the function onto the stack
        lua_getfield(_luaVmState.rawState(), -1, name.c_str());
        // push the metatable onto the stack again (now non-recusively)
        if (!lua_getmetatable(_luaVmState.rawState(), -2))
            cgtAssert(false, "This should not happen, Lua stack is corrupted!");

        // call the function
        _luaVmState.callLuaFunc(1, 0);

        // clean up the stack
        lua_pop(_luaVmState.rawState(), 1);
        _parent->popRecursive();
    }
    
    void MetatableLuaTable::pushField(const std::string& name) {
        LuaStateMutexType::scoped_lock lock(_luaVmState.getMutex());
        lua_State* L = _luaVmState.rawState();

        // load the parent field first
        _parent->pushField(_fieldName);

        // now load the metatable
        if (! lua_getmetatable(L, -1))
            cgtAssert(false, "This should not happen!");
        
        // now load the field
        auto it = _valueMap.find(name);
        if (it != _valueMap.end()) {
            lua_State* L = _luaVmState.rawState();

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

    void MetatableLuaTable::popRecursive() {
        LuaStateMutexType::scoped_lock lock(_luaVmState.getMutex());
        lua_pop(_luaVmState.rawState(), 2);
        _parent->popRecursive();
    }

    void MetatableLuaTable::populateValueMap() {
        _valueMap.clear();
        LuaStateMutexType::scoped_lock lock(_luaVmState.getMutex());
        lua_State* L = _luaVmState.rawState();
        
        // load this metatable onto stack
        _parent->pushField(_fieldName);
        if (! lua_getmetatable(L, -1))
            cgtAssert(false, "This should not happen!");

        iterateOverTableAndPopulateValueMap(L);
        
        lua_pop(L, 1);
        _parent->popRecursive();
    }

    LuaTable* MetatableLuaTable::getParentTable() {
        return _parent.get();
    }

}
