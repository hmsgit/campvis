#ifndef GLOBALLUATABLE_H__
#define GLOBALLUATABLE_H__

#include "luatable.h"
#include "luavmstate.h"


namespace campvis {

    class GlobalLuaTable : public LuaTable
    {
    public:
        GlobalLuaTable(LuaVmState& luaVmState);
        virtual ~GlobalLuaTable();

        virtual bool isValid();
        virtual std::shared_ptr<LuaTable> getTable(const std::string& name);
        virtual void callInstanceMethod(const std::string& name);

    protected:
        virtual void pushField(const std::string& name);
    };
}

#endif // GLOBALLUATABLE_H__
