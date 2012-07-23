#ifndef PIPELINEEVALUATOR_H__
#define PIPELINEEVALUATOR_H__

#include "sigslot/sigslot.h"
#include "core/pipeline/abstractpipeline.h"
#include "tbb/include/tbb/compat/condition_variable"

namespace TUMVis {

    /**
     * 
     */
    class PipelineEvaluator : public sigslot::has_slots<> {
    public:
        PipelineEvaluator(AbstractPipeline* pipeline);

        ~PipelineEvaluator();


        void startEvaluation();

        void stopEvaluation();

        void OnPipelineInvalidated();


    protected:

        AbstractPipeline* _pipeline;
        bool _evaluatePipeline;
        std::condition_variable _evaluationCondition;

        static const std::string loggerCat_;
    };

}

#endif // PIPELINEEVALUATOR_H__
