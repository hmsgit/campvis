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
