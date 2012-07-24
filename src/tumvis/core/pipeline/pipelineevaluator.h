#ifndef PIPELINEEVALUATOR_H__
#define PIPELINEEVALUATOR_H__

#include "sigslot/sigslot.h"
#include "core/pipeline/abstractpipeline.h"
#include "tbb/include/tbb/compat/thread"
#include "tbb/include/tbb/compat/condition_variable"
#include "tbb/include/tbb/atomic.h"

namespace TUMVis {

    /**
     * The PipelineEvaluator
     */
    class PipelineEvaluator : public sigslot::has_slots<> {
    public:
        PipelineEvaluator(AbstractPipeline* pipeline);

        ~PipelineEvaluator();


        void startEvaluation();
        void stopEvaluation();

        void evaluate();

        void OnPipelineInvalidated();


    protected:

        AbstractPipeline* _pipeline;
        tbb::atomic<bool> _evaluatePipeline;
        std::thread _evaluationThread;
        std::condition_variable _evaluationCondition;

        static const std::string loggerCat_;
    };

}

#endif // PIPELINEEVALUATOR_H__
