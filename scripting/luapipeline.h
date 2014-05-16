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
