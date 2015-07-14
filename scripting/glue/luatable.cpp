#include "luatable.h"
#include "globalluatable.h"
#include "regularluatable.h"
#include "luavmstate.h"

#include "cgt/assert.h"
#include "core/tools/stringutils.h"

extern "C" {
#include "lapi.h"
#include "lobject.h"
}


StkId index2addr(lua_State *L, int idx) {
    CallInfo *ci = L->ci;
    if (idx > 0) {
        TValue *o = ci->func + idx;
        api_check(L, idx <= ci->top - (ci->func + 1), "unacceptable index");
        if (o >= L->top) return 0;
        else return o;
    }
    else if (!(idx <= LUA_REGISTRYINDEX)) {  /* negative index */
        api_check(L, idx != 0 && -idx <= L->top - (ci->func + 1), "invalid index");
        return L->top + idx;
    }
    else if (idx == LUA_REGISTRYINDEX)
        return &G(L)->l_registry;
    else {  /* upvalues */
        idx = LUA_REGISTRYINDEX - idx;
        api_check(L, idx <= MAXUPVAL + 1, "upvalue index too large");
        if (ttislcf(ci->func))  /* light C function? */
            return 0;  /* it has no upvalues */
        else {
            CClosure *func = clCvalue(ci->func);
            return (idx <= func->nupvalues) ? &func->upvalue[idx - 1] : 0;
        }
    }
}

namespace campvis {

    LuaTable::LuaTable(LuaVmState& luaVmState) 
        : _luaVmState(luaVmState) 
    {}

    LuaTable::~LuaTable() {

    }

    std::shared_ptr<RegularLuaTable> LuaTable::getTable(const std::string& name) {
        // check whether this table has a field of given name and of type LUA_TTABLE
        auto it = _valueMap.find(name);
        if (it != _valueMap.end() && it->second.luaType == LUA_TTABLE) {
            // check whether the corresponding table needs to be initialized
            if (!it->second.luaTable) {
                it->second.luaTable = std::make_shared<RegularLuaTable>(this->shared_from_this(), name);
            }

            return it->second.luaTable;
        }
        else
            return std::shared_ptr<RegularLuaTable>(nullptr);
    }

    std::shared_ptr<MetatableLuaTable> LuaTable::getMetatable(const std::string& name) {
        // check whether this table has a field of given name, which has a metatable
        auto it = _valueMap.find(name);
        if (it != _valueMap.end() && it->second.hasMetatable) {
            // check whether the corresponding table needs to be initialized
            if (!it->second.luaMetatable)
                it->second.luaMetatable = std::make_shared<MetatableLuaTable>(this->shared_from_this(), name);

            return it->second.luaMetatable;
        }
        else
            return std::shared_ptr<MetatableLuaTable>(nullptr);
    }

    bool LuaTable::hasMetatable(const std::string& name) const {
        auto it = _valueMap.find(name);
        if (it != _valueMap.end())
            return it->second.hasMetatable;
        else
            return false;
    }

    size_t LuaTable::updateValueMap() {
        populateValueMap();
        return _valueMap.size();
    }
    
    const std::map<std::string, LuaTable::ValueStruct>& LuaTable::getValueMap() const {
        if (_valueMap.empty())
            const_cast<LuaTable*>(this)->populateValueMap();

        return _valueMap;
    }
    
    void LuaTable::iterateOverTableAndPopulateValueMap(lua_State* L) {
        void* tablePtr = hvalue(index2addr(L, -1));
        if (!getParentTable() || !getParentTable()->checkIfAlreadyDiscovered(tablePtr)) {
            _luaTablePointer = tablePtr;

            // iterate over table
            for (lua_pushnil(L); lua_next(L, -2); lua_pop(L, 1)) {
                int luaType = lua_type(L, -1);
                std::string name;
                bool keyIsNumber = false;
                bool hasMetatable = false;

                if (lua_type(L, -2) == LUA_TSTRING) {
                    name = lua_tostring(L, -2);
                }
                else if (lua_type(L, -2) == LUA_TNUMBER) {
                    continue;
                    name = StringUtils::toString(lua_tonumber(L, -2));
                    keyIsNumber = true;
                }
                else
                    cgtAssert(false, "Key of lua table is neither string nor number, This should not happen!");

                // check for metatable
                if (lua_getmetatable(L, -1)) {
                    hasMetatable = true;
                    lua_pop(L, 1);
                }

                ValueStruct vs = { luaType, nullptr, nullptr, keyIsNumber, hasMetatable };
                _valueMap.insert(std::make_pair(name, vs));

                if (luaType == LUA_TTABLE)
                    getTable(name);
            }
        }
        else {
            ValueStruct vs = { LUA_TNIL, nullptr, nullptr, false, false };
            _valueMap.insert(std::make_pair("...", vs));
        }
    }

    bool LuaTable::checkIfAlreadyDiscovered(void* luaTablePointer) {
        LuaTable* parent = getParentTable();
        return (_luaTablePointer == luaTablePointer) 
            || ((parent != nullptr) ? parent->checkIfAlreadyDiscovered(luaTablePointer) : false);
    }

}
