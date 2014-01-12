#include "luapipeline.h"

extern "C" {
#include "lualib.h"
#include "lauxlib.h"
}


namespace campvis {

    void callLuaFunc(lua_State* _luaState, int nargs, int nresults) {
        if (lua_pcall(_luaState, nargs, nresults, 0) != LUA_OK) {
            const char* errorMsg = lua_tostring(_luaState, -1);

            if (errorMsg == nullptr)
                std::cout << "(error object is not a string)" << std::endl;
            else
                std::cout << errorMsg << std::endl;

            lua_pop(_luaState, 1);
        }
    }

    LuaPipeline::LuaPipeline(std::string scriptPath, DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _scriptPath(scriptPath)
    {
        _luaState = luaL_newstate();

        // load the libs
        luaL_openlibs(_luaState);

        // run a Lua script here; true is returned if there were errors
        if (luaL_dofile(_luaState, scriptPath.c_str())) {
            const char* msg = lua_tostring(_luaState, -1);

            if (msg == nullptr)
                printf("(error object is not a string)");
            else
                printf("%s", msg);

            lua_pop(_luaState, 1);
        }

        lua_getglobal(_luaState, "pipeline");

        if (!lua_istable(_luaState, -1))
            printf("No valid Lua pipeline found (pipeline is %s)\n", lua_typename(_luaState, lua_type(_luaState, -1)));

        lua_pop(_luaState, 1);
    }

    LuaPipeline::~LuaPipeline() {
        lua_close(_luaState);
    }

    void LuaPipeline::init() {
        AutoEvaluationPipeline::init();

        lua_getglobal(_luaState, "pipeline");
        lua_getfield(_luaState, -1, "init");
        lua_getglobal(_luaState, "pipeline");
        callLuaFunc(_luaState, 1, 0);

        // Pop the pipeline table
        lua_pop(_luaState, 1);
    }

    void LuaPipeline::deinit() {
        AutoEvaluationPipeline::deinit();

        lua_getglobal(_luaState, "pipeline");
        lua_getfield(_luaState, -1, "deinit");
        lua_getglobal(_luaState, "pipeline");
        callLuaFunc(_luaState, 1, 0);

        // Pop the pipeline table
        lua_pop(_luaState, 1);
    }
}
