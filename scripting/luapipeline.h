#ifndef LUAPIPELINE_H__
#define LUAPIPELINE_H__

extern "C" {
#include "lua.h"
}

#include "core/pipeline/autoevaluationpipeline.h"


namespace campvis {
    class LuaPipeline : public AutoEvaluationPipeline {
    public:
        /**
         * Create a new Lua pipeline.
         *
         * \param scriptPath    Path to the Lua script defining the pipeline
         * \param dc            DataContainer containing local working set of data for this pipeline
         */
        LuaPipeline(const std::string scriptPath, DataContainer* dc);

        /**
         * Virtual Destructor
         */
        virtual ~LuaPipeline();

        /// \see AbstractPipeline::getName()
        virtual const std::string getName() const { return getId(); }
        static const std::string getId() { return "LuaPipeline"; }

        /// \see AutoEvaluationPipeline::init()
        virtual void init();

        /// \see AutoEvaluationPipeline::deinit()
        virtual void deinit();

    protected:
        const std::string _scriptPath;    ///< path to the Lua script defining the pipeline
        lua_State* _luaState;             ///< Lua state used to evaluate the pipeline
    };
}

#endif // LUAPIPELINE_H__
