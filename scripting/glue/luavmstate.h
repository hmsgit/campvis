#ifndef LUAVMSTATE_H__
#define LUAVMSTATE_H__

#include <memory>
#include <string>
#include "swigluarun.h"
#include "tbb/recursive_mutex.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}


namespace campvis {

    class GlobalLuaTable;

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

    class LuaVmState
    {
    public:
        LuaVmState(bool loadStdLibs = true);
        ~LuaVmState();

        bool execFile(const std::string& scriptPath);
        bool execString(const std::string& scriptString);

        std::shared_ptr<GlobalLuaTable> getGlobalTable();

        template<typename T>
        bool injectObjectPointer(T* objPointer, const std::string& typeName, const std::string& luaVarName);

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
         */
        void callLuaFunc(int nargs, int nresults);

    private:
        void logLuaError();

        lua_State* _luaState;               ///< Lua state managed by LuaVmState
        LuaStateMutexType _luaStateMutex;   ///< Mutex guarding access to the above Lua state
    };

    template<typename T>
    bool LuaVmState::injectObjectPointer(T* objPointer, const std::string& typeName, const std::string& luaVarName) {
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
    }
}

#endif // LUAVMSTATE_H__
