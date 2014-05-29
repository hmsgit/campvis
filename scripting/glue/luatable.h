#ifndef LUATABLE_H__
#define LUATABLE_H__

#include <memory>
#include <string>

#include "luavmstate.h"


namespace campvis {

    class RegularLuaTable;
    class GlobalLuaTable;

    /**
     * Base class for all Lua tables.
     *
     * LuaTable specifies the interface common to all concrete classes representing Lua tables.
     *
     * It's important to note that LuaTable and its subclasses are lazy: they don't access the Lua
     * VM unless it's necessary (e.g. on method call or field extraction). This helps to keep them
     * lightweight as they only have to know how to find the objects they correspond to in a Lua
     * state.
     *
     * Another defining characteristic of LuaTable and its subclasses is the fact that they need to
     * cooperate to access to the Lua VM. For instance, to call a function stored in a regular Lua
     * table, that table must first be extracted from the global Lua table. This is implemented by
     * calling pushField on the global table from the regular table. Nested regular tables require
     * such calls to be propagated through their enclosing tables all the way up to the global
     * table.
     */
    class LuaTable : private std::enable_shared_from_this<LuaTable>
    {
        friend RegularLuaTable;
        friend GlobalLuaTable;

    public:
        /**
         * Creates a new LuaTable.
         *
         * \param   luaVmState  Reference to the LuaVmState object from which the table originates
         */
        LuaTable(LuaVmState& luaVmState) : _luaVmState(luaVmState) {}

        /**
         * Virtual destructor.
         */
        virtual ~LuaTable() {}

        /**
         * Checks if this Lua table is valid.
         *
         * This method examines the table's associated Lua state to verify that it holds an object
         * corresponding to this Lua table.
         *
         * \return  true if this Lua table is valid, false otherwise
         */
        virtual bool isValid() = 0;

        /**
         * Returns a subtable of this Lua table.
         *
         * This method creates and returns an object representing a named Lua table nested in this
         * table.
         *
         * \param   name  Name of the subtable to return
         */
        virtual std::shared_ptr<LuaTable> getTable(const std::string& name) = 0;

        /**
         * Calls this table's instance method.
         *
         * callInstanceMethod invokes the specified function stored in this Lua table, passing it
         * the table as the first argument (instance).
         *
         * \param   name  Name of the instance method to call
         */
        virtual void callInstanceMethod(const std::string& name) = 0;

    protected:
        /**
         * Pushes a field of this Lua table onto the Lua VM's stack.
         *
         * This helper method is used to set up Lua's stack in order to access nested tables or
         * call methods.
         *
         * \param   name  Name of the field to push onto the stack
         */
        virtual void pushField(const std::string& name) = 0;

        LuaVmState& _luaVmState;   ///< Reference to the LuaVmState from which the table originates
    };
}

#endif // LUATABLE_H__
