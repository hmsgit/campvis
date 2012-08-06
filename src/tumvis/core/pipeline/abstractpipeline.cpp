#include "abstractpipeline.h"

#include "tgt/exception.h"
#include "core/pipeline/abstractprocessor.h"

namespace TUMVis {
    const std::string AbstractPipeline::loggerCat_ = "TUMVis.core.datastructures.AbstractPipeline";

    AbstractPipeline::AbstractPipeline() 
        : HasPropertyCollection()
    {
    }

    AbstractPipeline::~AbstractPipeline() {
    }

    void AbstractPipeline::init() {
        initAllProperties();

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

    void AbstractPipeline::deinit() {
        deinitAllProperties();

        // deinitialize all processors:
        for (std::vector<AbstractProcessor*>::iterator it = _processors.begin(); it != _processors.end(); ++it) {
            try {
                (*it)->deinit();
            }
            catch (tgt::Exception& e) {
                LERROR("Caught Exception during deinitialization of processor: " << e.what());
            }
        }
    }

    void AbstractPipeline::onPropertyChanged(const AbstractProperty* prop) {
        _invalidationLevel.setLevel(InvalidationLevel::INVALID_RESULT);
        s_PipelineInvalidated();
    }

    void AbstractPipeline::onProcessorInvalidated(const AbstractProcessor* processor) {
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

    InvalidationLevel& AbstractPipeline::getInvalidationLevel() {
        return _invalidationLevel;
    }

    const std::vector<AbstractProcessor*>& AbstractPipeline::getProcessors() const {
        return _processors;
    }

}
