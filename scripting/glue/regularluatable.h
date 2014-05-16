#ifndef REGULARLUATABLE_H__
#define REGULARLUATABLE_H__

#include "luatable.h"


namespace campvis {

    /**
     * Class representing regular Lua tables.
     *
     * Regular tables can be defined by exclusion: any Lua table that is not a global table is
     * a regular one. Tables are Lua's only built-in composite data type. Consequently, they serve
     * multiple purposes, including emulation of classes and objects.
     */
    class RegularLuaTable : public LuaTable
    {
    public:
        /**
         * Creates a new RegularLuaTable.
         *
         * \param   parent  Lua table (regular or global) that stores this table
         * \param   name    name of this table (key under which it is stored in its parent)
         */
        RegularLuaTable(std::shared_ptr<LuaTable> parent, std::string name);

        /**
         * Virtual destructor.
         */
        virtual ~RegularLuaTable();

        /// \see LuaTable::isValid()
        virtual bool isValid();

        /// \see LuaTable::getTable(const std::string&)
        virtual std::shared_ptr<LuaTable> getTable(const std::string& name);

        /// \see LuaTable::callInstanceMethod(const std::string&)
        virtual void callInstanceMethod(const std::string& name);

    protected:
        /// \see LuaTable::pushField(const std::string&)
        virtual void pushField(const std::string& name);

    private:
        std::shared_ptr<LuaTable> _parent;  ///< Lua table in which this table is stored
        std::string _name;                  ///< Name of this table
    };
}

#endif // REGULARLUATABLE_H__
