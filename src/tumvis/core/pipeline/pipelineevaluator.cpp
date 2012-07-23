#include "pipelineevaluator.h"

namespace TUMVis {
    const std::string PipelineEvaluator::loggerCat_ = "TUMVis.core.pipeline.PipelineEvaluator";


    PipelineEvaluator::PipelineEvaluator(AbstractPipeline* pipeline)
        : _pipeline(pipeline)
        , _evaluatePipeline(false)
    {
        tgtAssert(pipeline != 0, "Pipeline must not be 0.");
        pipeline->s_PipelineInvalidated.connect(this, &PipelineEvaluator::OnPipelineInvalidated);
    }

    PipelineEvaluator::~PipelineEvaluator() {
        _pipeline->s_PipelineInvalidated.disconnect(this);
    }

    void PipelineEvaluator::startEvaluation() {
        _evaluatePipeline = true;
        std::unique_lock<tbb::mutex> lock(_pipeline->getEvaluationMutex());

        while (_evaluatePipeline) {
            _pipeline->execute();

            while (_evaluatePipeline && _pipeline->getInvalidationLevel().isValid())
                _evaluationCondition.wait(lock);
        }
    }

    void PipelineEvaluator::stopEvaluation() {
        _evaluatePipeline = false;
        _evaluationCondition.notify_all();
    }

    void PipelineEvaluator::OnPipelineInvalidated() {
        if (_evaluatePipeline)
            _evaluationCondition.notify_all();
    }

}
