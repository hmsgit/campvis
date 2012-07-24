#ifndef PIPELINEEVALUATOR_H__
#define PIPELINEEVALUATOR_H__

#include "sigslot/sigslot.h"
#include "core/pipeline/abstractpipeline.h"
#include "core/tools/runnable.h"
#include "tbb/include/tbb/compat/thread"
#include "tbb/include/tbb/compat/condition_variable"
#include "tbb/include/tbb/atomic.h"

namespace TUMVis {

    /**
     * The PipelineEvaluator evaluates its pipeline in its own thread.
     * Evaluation is implemented using condidional wait - hence the pipeline is only evaluated when
     * \a pipeline emits the s_PipelineInvalidated signal.
     * \sa  Runnable
     */
    class PipelineEvaluator : public Runnable, public sigslot::has_slots<> {
    public:
        /**
         * Creates a new PipelineEvaluator for the given pipeline \a pipeline.
         * \param   pipeline    Pipeline to evaluate
         */
        PipelineEvaluator(AbstractPipeline* pipeline);

        /**
         * Destructor, stops and waits for the evaluation thread if it's still running.
         */
        ~PipelineEvaluator();

        /// \see Runnable::stop
        void stop();
        
        /**
         * Performs the pipeline evaluation using conditional wait.
         * \sa Runnable::run
         */
        void run();

        /**
         * Slot for notifications when the pipeline was invalidated.
         */
        void OnPipelineInvalidated();

    protected:
        AbstractPipeline* _pipeline;                    ///< Pipeline to evaluate
        std::condition_variable _evaluationCondition;   ///< conditional wait condition
    };

}

#endif // PIPELINEEVALUATOR_H__
