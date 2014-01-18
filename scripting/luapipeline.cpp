#include "luapipeline.h"

extern "C" {
#include "lualib.h"
#include "lauxlib.h"
}

#include "swigluarun.h"


namespace campvis {

    void LuaPipeline::callLuaFunc(lua_State* _luaState, int nargs, int nresults) {
        if (lua_pcall(_luaState, nargs, nresults, 0) != LUA_OK) {
            this->logLuaError();
        }
    }

    LuaPipeline::LuaPipeline(std::string scriptPath, DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _scriptPath(scriptPath)
    {
        _luaState = luaL_newstate();

        // load the libs
        luaL_openlibs(_luaState);

/*
 * Defined if CAMPVis Lua modules are placed in a location that won't be picked up by Lua
 * automatically
 */
#ifdef CAMPVIS_LUA_MODS_PATH

#define STRINGIFY(x) #x
#define TOSTRING(x) STRINGIFY(x)

        /* Let Lua know where CAMPVis modules are located */
        if (luaL_dostring(_luaState, "package.cpath = package.cpath .. ';" TOSTRING(CAMPVIS_LUA_MODS_PATH) "'")) {
            this->logLuaError();
            return;
        }

#undef TOSTRING
#undef STRINGIFY

#endif // CAMPVIS_LUA_MODS_PATH

        if (luaL_dostring(_luaState, "require(\"campvis\")")) {
            this->logLuaError();
            return;
        }

        swig_type_info* autoEvaluationPipelineType = SWIG_TypeQuery(_luaState, "campvis::AutoEvaluationPipeline *");

        if (autoEvaluationPipelineType == nullptr)
            printf("SWIG wrapper for campvis::AutoEvaluationPipeline not found");
        else {
            SWIG_NewPointerObj(_luaState, this, autoEvaluationPipelineType, 0);
            lua_setglobal(_luaState, "instance");
        }

        // run a Lua script here; true is returned if there were errors
        if (luaL_dofile(_luaState, scriptPath.c_str())) {
            this->logLuaError();
            return;
        }

        lua_getglobal(_luaState, "pipeline");

        if (!lua_istable(_luaState, -1))
            printf("No valid Lua pipeline found (pipeline is %s)\n", lua_typename(_luaState, lua_type(_luaState, -1)));
        else {
            lua_getfield(_luaState, -1, "ctor");
            lua_getglobal(_luaState, "pipeline");
            callLuaFunc(_luaState, 1, 0);
        }

        // Pop the pipeline table
        lua_pop(_luaState, 1);
    }

    LuaPipeline::~LuaPipeline() {
        lua_close(_luaState);
    }

    void LuaPipeline::logLuaError() {
        const char* errorMsg = lua_tostring(_luaState, -1);

        if (errorMsg == nullptr)
            std::cerr << "(error object is not a string)" << std::endl;
        else
            std::cerr << errorMsg << std::endl;

        lua_pop(_luaState, 1);
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
