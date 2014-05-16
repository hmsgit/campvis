#ifndef SCRIPTEDPIPELINEREGISTRAR_H__
#define SCRIPTEDPIPELINEREGISTRAR_H__

#include "luapipeline.h"
#include "modules/pipelinefactory.h"


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
