#include "pipelineevaluator.h"
#include "tgt/qt/qtcontextmanager.h"
#include "core/pipeline/abstractpipeline.h"

namespace TUMVis {
    PipelineEvaluator::PipelineEvaluator(AbstractPipeline* pipeline)
        : Runnable()
        , _pipeline(pipeline)
    {
        tgtAssert(pipeline != 0, "Pipeline must not be 0.");
        pipeline->s_PipelineInvalidated.connect(this, &PipelineEvaluator::OnPipelineInvalidated);
    }

    PipelineEvaluator::~PipelineEvaluator() {
        _pipeline->s_PipelineInvalidated.disconnect(this);
    }

    void PipelineEvaluator::run() {
        std::unique_lock<tbb::mutex> lock(_pipeline->_evaluationMutex);

        while (! _stopExecution) {
            _pipeline->execute();

            while (!_stopExecution && _pipeline->getInvalidationLevel().isValid())
                _evaluationCondition.wait(lock);
        }

        // release OpenGL context, so that other threads can access it
        CtxtMgr.releaseCurrentContext();
    }

    void PipelineEvaluator::stop() {
        // we need to execute run() one more time to ensure correct release of the OpenGL context
        _stopExecution = true;
        _evaluationCondition.notify_all();

        Runnable::stop();
    }

    void PipelineEvaluator::OnPipelineInvalidated() {
        if (!_stopExecution)
            _evaluationCondition.notify_all();
    }

}