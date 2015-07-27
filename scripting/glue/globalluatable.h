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

#ifndef GLOBALLUATABLE_H__
#define GLOBALLUATABLE_H__

#include "luatable.h"
#include "scripting/scriptingapi.h"

namespace campvis {

    /**
     * Class representing global Lua tables.
     *
     * Global tables store references to all objects that live in the global scope of a Lua VM. As
     * a result, they must be used in order to interact with Lua states and access user-defined
     * values.
     */
    class CAMPVIS_SCRIPTING_API GlobalLuaTable : public LuaTable {
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

        virtual bool isValid() override;
        virtual void callInstanceMethod(const std::string& name) override;

        virtual void pushField(const std::string& name) override;
        virtual void popRecursive() override;
        
    protected:
        virtual void populateValueMap() override;
        virtual LuaTable* getParentTable() override;
    };

}

#endif // GLOBALLUATABLE_H__
