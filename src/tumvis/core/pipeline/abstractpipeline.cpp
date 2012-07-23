#include "abstractpipeline.h"
#include "tgt/exception.h"

namespace TUMVis {
    const std::string AbstractPipeline::loggerCat_ = "TUMVis.core.datastructures.AbstractPipeline";

    AbstractPipeline::AbstractPipeline() 
    {
    }

    AbstractPipeline::~AbstractPipeline() {
    }

    PropertyCollection& AbstractPipeline::getPropertyCollection() {
        return _properties;
    }

    void AbstractPipeline::init() {
        // initialize all processors:
         for (std::vector<AbstractProcessor*>::iterator it = _processors.begin(); it != _processors.end(); ++it) {
             try {
                 (*it)->init();
             }
             catch (tgt::Exception& e) {
                 LERROR("Caught Exception during initialization of processor: " << e.what());
             }
         }
    }

    void AbstractPipeline::onNotify(const ProcessorObserverArgs& poa) {
        _invalidationLevel.setLevel(InvalidationLevel::INVALID_RESULT);
        s_PipelineInvalidated();
    }

    const DataContainer& AbstractPipeline::getDataContainer() const {
        return _data;
    }

    void AbstractPipeline::executeProcessor(AbstractProcessor& processor) {
        processor.lockProperties();
        processor.process(_data);
        processor.unlockProperties();
    }

    tbb::mutex& AbstractPipeline::getEvaluationMutex() {
        return _evaluationMutex;
    }

    InvalidationLevel& AbstractPipeline::getInvalidationLevel() {
        return _invalidationLevel;
    }

}
