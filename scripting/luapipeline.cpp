#include "luapipeline.h"

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}

#include "glue/globalluatable.h"
#include "glue/luavmstate.h"


namespace campvis {

    LuaPipeline::LuaPipeline(const std::string name, std::string scriptPath, DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _name(name)
        , _scriptPath(scriptPath)
        , _luaVmState(new LuaVmState())
        , _pipelineTable(_luaVmState->getGlobalTable()->getTable("pipeline"))
    {
        // Let Lua know where CAMPVis modules are located
        if (!_luaVmState->execString("package.cpath = '" CAMPVIS_LUA_MODS_PATH "'"))
            return;

        // Load CAMPVis' core Lua module to have SWIG glue for AutoEvaluationPipeline available
        if (!_luaVmState->execString("require(\"campvis\")"))
            return;

        // Make this pipeline object available to the script so that it can configure it
        if (!_luaVmState->injectObjectPointer(this, "campvis::AutoEvaluationPipeline *", "instance"))
            return;

        // Try executing the pipeline's Lua script
        if (!_luaVmState->execFile(scriptPath))
            return;

        if (!_pipelineTable->isValid())
            std::cerr << "No valid Lua pipeline found (global variable `pipeline` is not a table)" << std::endl;
        else
            _pipelineTable->callInstanceMethod("ctor");
    }

    LuaPipeline::~LuaPipeline() {
    }

    void LuaPipeline::init() {
        AutoEvaluationPipeline::init();
        _pipelineTable->callInstanceMethod("init");
    }

    void LuaPipeline::deinit() {
        AutoEvaluationPipeline::deinit();
        _pipelineTable->callInstanceMethod("deinit");
    }
}
