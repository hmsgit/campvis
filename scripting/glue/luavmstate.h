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

#ifndef LUAVMSTATE_H__
#define LUAVMSTATE_H__

#include <iostream>
#include <memory>
#include <string>
#include "scripting/swigluarun.h"
#include "scripting/glue/globalluatable.h"
#include "tbb/recursive_mutex.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}


namespace campvis {

    /**
     * The Lua state managed by LuaVmState has to be protected with a mutex because it's not
     * thread-safe and different threads (running LuaPipeline or a processor attached to it) may try
     * to access it simultaneously.
     *
     * The mutex needs to be recursive due to the fact that Lua code can trigger the emission (or
     * copying) of signals that have slots defined it Lua connected to them. This in turn causes the
     * state to be accessed from a thread that, unbeknownst to it, already holds a lock on the
     * mutex.
     */
    typedef tbb::recursive_mutex LuaStateMutexType;

    /**
     * Class used to create and manage a Lua state.
     *
     * LuaVmState provides a thin wrapper around Lua's struct lua_State. It implements helpers for
     * common operations (e.g. script execution) while still giving access to the underlying raw
     * Lua state.
     */
    class LuaVmState
    {
    public:
        /**
         * Creates a new LuaVmState.
         *
         * \param   loadStdLibs  Should standard Lua libraries be loaded into the state created by
         *                       LuaVmState?
         */
        LuaVmState(bool loadStdLibs = true);

        /**
         * Virtual destructor.
         */
        ~LuaVmState();

        /**
         * Executes a Lua script in the context of this VM.
         *
         * \param   scriptPath   Path to a file containing the Lua script to execute
         * \return  true if the script could be located and executed successfully, false otherwise
         */
        bool execFile(const std::string& scriptPath);

        /**
         * Executes a Lua script in the context of this VM.
         *
         * \param   scriptString   String containing the Lua script to execute
         * \return  true if the script could be executed successfully, false otherwise
         */
        bool execString(const std::string& scriptString);

        /**
         * Returns the global table of the Lua state managed by LuaVmState.
         */
        std::shared_ptr<GlobalLuaTable> getGlobalTable();

        void redirectLuaPrint();

        /**
         * Wraps a C++ object using SWIG and injects it into the Lua state managed by LuaVmState.
         *
         * SWIG glue for the object to be injected must be loaded (using execFile or execString)
         * into the Lua state before calling this method or the injection will fail. The Lua VM
         * doesn't take ownership of the object.
         *
         * \param   objPointer   Pointer to the C++ object to inject into the Lua state
         * \param   typeName     SWIG name of the object's type
         * \param   luaVarName   Name under which to store the object in the Lua state
         * \return  true if the object was injected into the Lua state successfully, false otherwise
         */
        template<typename T>
        bool injectGlobalObjectPointer(T* objPointer, const std::string& typeName, const std::string& luaVarName);

        /**
         * Wraps a C++ object using SWIG and injects it into the Lua state managed by LuaVmState.
         *
         * SWIG glue for the object to be injected must be loaded (using execFile or execString)
         * into the Lua state before calling this method or the injection will fail. The Lua VM
         * doesn't take ownership of the object.
         *
         * \param   objPointer      Pointer to the C++ object to inject into the Lua state
         * \param   typeName        SWIG name of the object's type
         * \param   luaTableName    Table name in which to store the object in the Lua state
         * \param   luaTableIndex   Index in the table to store the object in
         * \return  true if the object was injected into the Lua state successfully, false otherwise
         */
        template<typename T>
        bool injectObjectPointerToTable(T* objPointer, const std::string& typeName, const std::string& luaTableName, int luaTableIndex);

        /**
         * Wraps a C++ object using SWIG and injects it into the Lua state managed by LuaVmState.
         *
         * SWIG glue for the object to be injected must be loaded (using execFile or execString)
         * into the Lua state before calling this method or the injection will fail. The Lua VM
         * doesn't take ownership of the object.
         *
         * \param   objPointer      Pointer to the C++ object to inject into the Lua state
         * \param   typeName        SWIG name of the object's type
         * \param   luaTableName    Table name in which to store the object in the Lua state
         * \param   luaTableIndex   Index in the table to store the object in
         * \return  true if the object was injected into the Lua state successfully, false otherwise
         */
        template<typename T>
        bool injectObjectPointerToTableField(T* objPointer, const std::string& typeName, const std::string& luaTableName, const std::string& luaFieldName);

        /**
         * Return the Lua state managed by LuaVmState.
         *
         * \note The mutex returned by getMutex() must be locked before accessing the state in any
         *       way.
         */
        lua_State* rawState() const;

        /**
         * Return the mutex guarding access to the Lua state managed by LuaVmState.
         */
        LuaStateMutexType& getMutex();

        /**
         * Call the Lua function that's at the top of the stack.
         *
         * The caller is responsible for setting up the Lua VM in preparation of the call by
         * placing the function and all of its arguments on Lua's stack.
         *
         * \param   nargs     Number of arguments to pass to the function
         * \param   nresults  Number of expected results
         */
        void callLuaFunc(int nargs, int nresults);

    private:
        /**
         * Logs a Lua error to stderr.
         */
        void logLuaError();

        lua_State* _luaState;                               ///< Lua state managed by LuaVmState
        std::shared_ptr<GlobalLuaTable> _globalLuaTable;    ///< Pointer to global Lua table of this VM
        LuaStateMutexType _luaStateMutex;                   ///< Mutex guarding access to the above Lua state
    };

    template<typename T>
    bool LuaVmState::injectGlobalObjectPointer(T* objPointer, const std::string& typeName, const std::string& luaVarName) {
        LuaStateMutexType::scoped_lock lock(_luaStateMutex);
        swig_type_info* objTypeInfo = SWIG_TypeQuery(_luaState, typeName.c_str());

        if (objTypeInfo == nullptr) {
            std::cerr << "SWIG wrapper for " << typeName << " not found" << std::endl;
            return false;
        } else {
            SWIG_NewPointerObj(_luaState, objPointer, objTypeInfo, 0);
            lua_setglobal(_luaState, luaVarName.c_str());
            return true;
        }
    };

    template<typename T>
    bool LuaVmState::injectObjectPointerToTable(T* objPointer, const std::string& typeName, const std::string& luaTableName, int luaTableIndex) {
        LuaStateMutexType::scoped_lock lock(_luaStateMutex);
        swig_type_info* objTypeInfo = SWIG_TypeQuery(_luaState, typeName.c_str());

        if (objTypeInfo == nullptr) {
            std::cerr << "SWIG wrapper for " << typeName << " not found" << std::endl;
            return false;
        } else {
            lua_getglobal(_luaState, luaTableName.c_str());             // push the table name
            lua_pushnumber(_luaState, luaTableIndex);                   // push the index
            SWIG_NewPointerObj(_luaState, objPointer, objTypeInfo, 0);  // push the object
            lua_settable(_luaState, -3);                                // set the table in the VM
            lua_pop(_luaState, 1);
            return true;
        }
    };

    template<typename T>
    bool LuaVmState::injectObjectPointerToTableField(T* objPointer, const std::string& typeName, const std::string& luaTableName, const std::string& luaFieldName) {
        LuaStateMutexType::scoped_lock lock(_luaStateMutex);
        swig_type_info* objTypeInfo = SWIG_TypeQuery(_luaState, typeName.c_str());

        if (objTypeInfo == nullptr) {
            std::cerr << "SWIG wrapper for " << typeName << " not found" << std::endl;
            return false;
        } else {
            lua_getglobal(_luaState, luaTableName.c_str());             // push the table name
            lua_pushstring(_luaState, luaFieldName.c_str());            // push the field name
            SWIG_NewPointerObj(_luaState, objPointer, objTypeInfo, 0);  // push the object
            lua_settable(_luaState, -3);                                // set the table in the VM
            lua_pop(_luaState, 1);
            return true;
        }
    };

}

#endif // LUAVMSTATE_H__
