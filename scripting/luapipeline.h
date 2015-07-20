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

#ifndef LUAPIPELINE_H__
#define LUAPIPELINE_H__

#include <memory>
#include "core/pipeline/autoevaluationpipeline.h"


namespace campvis {
    class LuaTable;
    class LuaVmState;

    /**
     * Class representing CAMPVis pipelines defined in Lua.
     */
    class LuaPipeline : public AutoEvaluationPipeline {
    public:
        /**
         * Create a new Lua pipeline.
         *
         * \param name          Name of this pipeline
         * \param scriptPath    Path to the Lua script defining the pipeline
         * \param dc            DataContainer containing local working set of data for this pipeline
         */
        LuaPipeline(const std::string name, const std::string scriptPath, DataContainer* dc);

        /**
         * Virtual Destructor
         */
        virtual ~LuaPipeline();

        /// \see AbstractPipeline::getName()
        virtual const std::string getName() const { return _name; }

        /// \see AutoEvaluationPipeline::init()
        virtual void init();

        /// \see AutoEvaluationPipeline::deinit()
        virtual void deinit();

    protected:
        const std::string _name;                     ///< The name of this pipeline
        const std::string _scriptPath;               ///< Path to the Lua script defining the pipeline
        LuaVmState* _luaVmState;                     ///< Lua VM state used to evaluate the pipeline
        std::shared_ptr<LuaTable> _pipelineTable;    ///< Pointer to the Lua table associated with the pipeline
    };
}

#endif // LUAPIPELINE_H__
