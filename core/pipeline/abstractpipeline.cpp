// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#include "abstractpipeline.h"

#include "tgt/exception.h"
#include "core/pipeline/abstractprocessor.h"
#include <ctime>


namespace campvis {
    const std::string AbstractPipeline::loggerCat_ = "CAMPVis.core.datastructures.AbstractPipeline";

    AbstractPipeline::AbstractPipeline() 
        : HasPropertyCollection()
        , _enabled(true)
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

    void AbstractPipeline::onProcessorInvalidated(AbstractProcessor* processor) {
        _invalidationLevel.setLevel(InvalidationLevel::INVALID_RESULT);
        s_PipelineInvalidated();
    }

    const DataContainer& AbstractPipeline::getDataContainer() const {
        return _data;
    }

    DataContainer& AbstractPipeline::getDataContainer() {
        return _data;
    }

    void AbstractPipeline::executeProcessor(AbstractProcessor* processor) {
        tgtAssert(processor != 0, "Processor must not be 0.");

        if (processor->getEnabled() && !processor->getInvalidationLevel().isValid()) {
            processor->lockProperties();
#ifdef CAMPVIS_DEBUG
            clock_t startTime = clock();
#endif
            processor->process(_data);
#ifdef CAMPVIS_DEBUG
            clock_t endTime = clock();
#endif
            processor->unlockProperties();

#ifdef CAMPVIS_DEBUG
            LDEBUG("Executed processor " << processor->getName() << " duration: " << (endTime - startTime));
#endif
        }
    }

    InvalidationLevel& AbstractPipeline::getInvalidationLevel() {
        return _invalidationLevel;
    }

    const std::vector<AbstractProcessor*>& AbstractPipeline::getProcessors() const {
        return _processors;
    }

    bool AbstractPipeline::getEnabled() const {
        return _enabled;
    }

    void AbstractPipeline::setEnabled(bool enabled) {
        _enabled = enabled;
    }

    void AbstractPipeline::addProcessor(AbstractProcessor* processor) {
        _processors.push_back(processor);
    }
}
