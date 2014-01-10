#ifndef LUAPIPELINE_H__
#define LUAPIPELINE_H__

extern "C" {
#include "lua.h"
}

#include "core/pipeline/autoevaluationpipeline.h"


namespace campvis {
    class LuaPipeline : public AutoEvaluationPipeline {
    public:
        LuaPipeline(lua_State* state, DataContainer* dc);

        /// \see AbstractPipeline::getName()
        virtual const std::string getName() const { return getId(); }
        static const std::string getId() { return "LuaPipeline"; }

        /// \see AutoEvaluationPipeline::init()
        virtual void init();

        /// \see AutoEvaluationPipeline::deinit()
        virtual void deinit();

    protected:
        lua_State* _luaState;
    };
}

#endif // LUAPIPELINE_H__
