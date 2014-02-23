#ifndef LUATABLE_H__
#define LUATABLE_H__

#include <memory>
#include <string>

#include "luavmstate.h"


namespace campvis {

    class RegularLuaTable;
    class GlobalLuaTable;

    class LuaTable : private std::enable_shared_from_this<LuaTable>
    {
        friend RegularLuaTable;
        friend GlobalLuaTable;

    public:
        LuaTable(LuaVmState& luaVmState) : _luaVmState(luaVmState) {}
        virtual ~LuaTable() {}

        virtual bool isValid() = 0;
        virtual std::shared_ptr<LuaTable> getTable(const std::string& name) = 0;
        virtual void callInstanceMethod(const std::string& name) = 0;

    protected:
        virtual void pushField(const std::string& name) = 0;

        LuaVmState& _luaVmState;
    };
}

#endif // LUATABLE_H__
