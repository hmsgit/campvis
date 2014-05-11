#ifndef GLOBALLUATABLE_H__
#define GLOBALLUATABLE_H__

#include "luatable.h"
#include "luavmstate.h"


namespace campvis {

    /**
     * Class representing global Lua tables.
     *
     * Global tables store references to all objects that live in the global scope of a Lua VM. As
     * a result, they must be used in order to interact with Lua states and access user-defined
     * values.
     */
    class GlobalLuaTable : public LuaTable
    {
    public:
        /**
         * Creates a new GlobalLuaTable.
         *
         * \param   luaVmState  Reference to the LuaVmState object from which the table originates
         */
        GlobalLuaTable(LuaVmState& luaVmState);

        /**
         * Virtual destructor.
         */
        virtual ~GlobalLuaTable();

        /// \see LuaTable::isValid()
        virtual bool isValid();

        /// \see LuaTable::getTable(const std::string&)
        virtual std::shared_ptr<LuaTable> getTable(const std::string& name);

        /// \see LuaTable::callInstanceMethod(const std::string&)
        virtual void callInstanceMethod(const std::string& name);

    protected:
        /// \see LuaTable::pushField(const std::string&)
        virtual void pushField(const std::string& name);
    };
}

#endif // GLOBALLUATABLE_H__
