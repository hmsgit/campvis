// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#ifndef LUATABLE_H__
#define LUATABLE_H__

#include <map>
#include <memory>
#include <set>
#include <string>

struct lua_State;

namespace campvis {
    class LuaVmState;
    class GlobalLuaTable;
    class MetatableLuaTable;
    class RegularLuaTable;

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
    class LuaTable : public std::enable_shared_from_this<LuaTable>
    {
        friend GlobalLuaTable;
        friend MetatableLuaTable;
        friend RegularLuaTable;

    public:
        /// Struct storing information about a single value in this Lua table
        struct ValueStruct {
            /// Lua type of the value
            int luaType;

            /// If the value is of type LUA_TTABLE, carries the pointer to corresponding LuaTable object. 
            /// However, this variable is lazy-initialized.
            std::shared_ptr<RegularLuaTable> luaTable;

            /// If the value has a metatable, carries the pointer to the corresponding MetatableLuaTable object.
            /// However, this variable is lazy-initialized. You can check for existing metatable (without 
            /// instantiating one) with the hasMetatable field.
            std::shared_ptr<MetatableLuaTable> luaMetatable;

            /// Flag whether the corresponding key should be treated as number instead of as string. 
            /// This avoids confusing lua_next() during table iteration when implicitly converting number
            /// keys into string keys through calling lua_tostring().
            /// This would acutally better fit into the key, but then map lookup would be more troublesome.
            bool keyIsNumber;

            /// Flag whether this field has a Lua metatable. This flag allows for a cheap check for a 
            /// metatable without forcing to actually lazy-instantiate the MetatableLuaTable object.
            bool hasMetatable;
        };

        /**
         * Creates a new LuaTable.
         *
         * \param   luaVmState  Reference to the LuaVmState object from which the table originates
         */
        LuaTable(LuaVmState& luaVmState);

        /**
         * Virtual destructor.
         */
        virtual ~LuaTable();

        /**
         * Returns the LuaVmState of this table
         * \return  _luaVmState
         */
        LuaVmState& getLuaVmState() { return _luaVmState; }

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
         * Returns a subtable of this Lua table if existent.
         *
         * If this table has a subtable with the given name, it returns a pointer to the corresponding
         * RegularLuaTable object. This is lazy-instantiated if needed. If no such field exists, 
         * a nullptr is returned.
         *
         * \param   name  Name of the subtable to return.
         * \return  A pointer to a RegularLuaTable object for the given subtable; nullptr if no such 
         *          table exists.
         */
        std::shared_ptr<RegularLuaTable> getTable(const std::string& name);

        /**
         * Returns the Lua metatable of the given field if existent.
         *
         * If this table has a field with the given name, which has a metatable, it returns a pointer 
         * to the corresponding MetatableLuaTable object. This is lazy-instantiated if needed. 
         * If no such metatable exists, a nullptr is returned.
         *
         * \param   name  Name of the field for which to return the metatable.
         * \return  A pointer to the MetatableLuaTable object for the given field; nullptr if no 
         *          metatable exists.
         */
        std::shared_ptr<MetatableLuaTable> getMetatable(const std::string& name);

        /**
         * Checks whether the given field has a Lua metatable. 
         * 
         * This method allows for checking for metatable without forcing to lazy-instantiate 
         * the MetatableLuaTable object as it would be done through getMetatable(name)
         * 
         * \param   name  Name of the field for which to return the metatable.
         * \return  True if the corresponding field has a metatable
         */
        bool hasMetatable(const std::string& name) const;

        /**
         * Calls this table's instance method.
         *
         * callInstanceMethod invokes the specified function stored in this Lua table, passing it
         * the table as the first argument (instance).
         *
         * \param   name  Name of the instance method to call
         */
        virtual void callInstanceMethod(const std::string& name) = 0;
        
        /**
         * Updates the value map caching the current state of this table.
         * \return  The number of fields of this Lua table.
         */
        virtual size_t updateValueMap();


        /**
         * Pushes a field of this Lua table onto the Lua VM's stack.
         * This helper method is used to set up Lua's stack in order to access nested tables or
         * call methods.
         *
         * \param   name  Name of the field to push onto the stack
         */
        virtual void pushField(const std::string& name) = 0;

        /**
         * When calling pushField() or pushMetatable() on deep table hierarchies, the Lua stack is 
         * populated with one value for each level. popRecursive() takes care of cleaning up the 
         * Lua stack according to the table hierarchy. It pops as many values from the stack as 
         * there were pushed during pushField().
         */
        virtual void popRecursive() = 0;

        /**
         * Returns the value map of this table.
         * You can use it to traverse over the fields of this table.
         * \return  _valueMap
         */
        const std::map<std::string, ValueStruct>& getValueMap() const;

    protected:
        /**
         * Populates _valueMap with the contents of this Lua table.
         */
        virtual void populateValueMap() = 0;

        /**
         * The actual Lua table traversal for populating the value map is the same for all table types.
         * Therefore, we have this base method here, which can be called from the child classes
         * after they successfully set up the stack.
         * 
         * \param L     Lua state to use during table traversal.
         */
        void iterateOverTableAndPopulateValueMap(lua_State* L);

        /**
         * Returns this LuaTable's parent LuaTable.
         * To be implemented in sub classes. Used to detect cyclic table loops.
         */
        virtual LuaTable* getParentTable() = 0;

        /**
         * Checks whether the Lua table with the given Lua VM raw pointer has already been discovered.
         * Use this method to detect cyclic table loops. It recursively checks all parent tables 
         * whether they correspond to the given raw pointer.
         * \param   luaTablePointer Raw pointer to the Lua table in the Lua VM.
         * \return  True if one of the parents of corresponds to the given Lua VM raw pointer.
         */
        bool checkIfAlreadyDiscovered(void* luaTablePointer);


        /// Reference to the LuaVmState from which the table originates
        LuaVmState& _luaVmState;

        /// Raw pointer to the Lua table in the Lua VM. Used to detect cyclic table loops.
        void* _luaTablePointer;

        /// value map of this lua table, mirroring the contents.
        mutable std::map<std::string, ValueStruct> _valueMap;

        /// Flag whether the elements in this table have already been discovered (i.e. whether _valueMap was populated)
        mutable bool _tableDiscovered;
    };

}

// we always need these three types when handling with LuaTables, so include their definitions here
#include "metatableluatable.h"
#include "regularluatable.h"
#include "globalluatable.h"

#endif // LUATABLE_H__
