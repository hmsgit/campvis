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

#ifndef REGULARLUATABLE_H__
#define REGULARLUATABLE_H__

#include "luatable.h"
#include "scripting/scriptingapi.h"

namespace campvis {

    /**
     * Class representing regular Lua tables.
     *
     * Regular tables can be defined by exclusion: any Lua table that is not a global table or a
     * metatable is a regular one. 
     */
    class CAMPVIS_SCRIPTING_API RegularLuaTable : public LuaTable {
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

        virtual bool isValid() override;
        virtual void callInstanceMethod(const std::string& name) override;
        
        virtual void pushField(const std::string& name) override;
        virtual void popRecursive() override;

    protected:
        virtual void populateValueMap() override;
        virtual LuaTable* getParentTable() override;

    private:
        std::shared_ptr<LuaTable> _parent;  ///< Lua table in which this table is stored.
        std::string _name;                  ///< Name of this table in _parent.
    };
}

#endif // REGULARLUATABLE_H__
