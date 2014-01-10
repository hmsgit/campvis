#include "luapipeline.h"


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

    LuaPipeline::LuaPipeline(lua_State* state, DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _luaState(state)
    {
        lua_getglobal(state, "pipeline");

        if (!lua_istable(state, -1))
            printf("No valid Lua pipeline found (pipeline is %s)\n", lua_typename(state, lua_type(state, -1)));

        lua_pop(state, 1);
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
