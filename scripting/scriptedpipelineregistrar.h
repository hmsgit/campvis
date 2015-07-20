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

#ifndef SCRIPTEDPIPELINEREGISTRAR_H__
#define SCRIPTEDPIPELINEREGISTRAR_H__

#include "luapipeline.h"
#include "core/pipeline/pipelinefactory.h"


namespace campvis {

    /**
     * PipelineFactory requires each pipeline to be represented by a separate class whose static
     * getId method returns a unique identifier. As all Lua pipelines are instance of one class,
     * LuaPipeline, and only differ with respect to the script they execute (which is not known at
     * compile time), the following adapter class is needed.
     *
     * IdentifiableLuaPipeline gets its identifier as a template argument, and is able to return it
     * from a static method as a result.
     */
    template <const char* id>
    class IdentifiableLuaPipeline : public LuaPipeline {
    public:
        IdentifiableLuaPipeline(const std::string scriptPath, DataContainer* dc) : LuaPipeline(id, scriptPath, dc) {}

        static const std::string getId() { return id; }
    };

// ================================================================================================

    template<typename T, const char* scriptPath>
    class ScriptedPipelineRegistrar {
    public:
        /**
         * Static factory method for creating the pipeline of type T.
         * \param   dc  DataContainer for the created pipeline to work on.
         * \return  A newly created Lua pipeline of type T. Caller has to take ownership of the pointer.
         */
        static AbstractPipeline* create(DataContainer* dc) {
            return new T(scriptPath, dc);
        }

    private:
        /// static helper field to ensure registration at static initialization time.
        static const size_t _factoryId;
    };

    template<typename T, const char* scriptPath>
    const size_t ScriptedPipelineRegistrar<T, scriptPath>::_factoryId =
            PipelineFactory::getRef().registerPipeline<T>(&ScriptedPipelineRegistrar<T, scriptPath>::create);
}

#endif // SCRIPTEDPIPELINEREGISTRAR_H__
