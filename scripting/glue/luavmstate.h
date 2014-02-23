#ifndef LUAVMSTATE_H__
#define LUAVMSTATE_H__

#include <memory>
#include <string>
#include "swigluarun.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

struct lua_State;


namespace campvis {

    class GlobalLuaTable;

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
        struct lua_State* rawState() const;

        /**
         * Call the Lua function that's at the top of the stack.
         */
        void callLuaFunc(int nargs, int nresults);

    private:
        void logLuaError();

        struct lua_State* _luaState;      ///< Lua state managed by LuaVmState
    };

    template<typename T>
    bool LuaVmState::injectObjectPointer(T* objPointer, const std::string& typeName, const std::string& luaVarName) {
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
