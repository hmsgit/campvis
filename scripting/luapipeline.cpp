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
        if (!_luaVmState->injectGlobalObjectPointer(this, "campvis::AutoEvaluationPipeline *", "instance"))
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
