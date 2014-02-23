#ifndef REGULARLUATABLE_H__
#define REGULARLUATABLE_H__

#include "luatable.h"


namespace campvis {

    class RegularLuaTable : public LuaTable
    {
    public:
        RegularLuaTable(std::shared_ptr<LuaTable> parent, std::string name);
        virtual ~RegularLuaTable();

        virtual bool isValid();
        virtual std::shared_ptr<LuaTable> getTable(const std::string& name);
        virtual void callInstanceMethod(const std::string& name);

    protected:
        virtual void pushField(const std::string& name);

    private:
        std::shared_ptr<LuaTable> _parent;
        std::string _name;
    };
}

#endif // REGULARLUATABLE_H__
