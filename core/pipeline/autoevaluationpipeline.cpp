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

#include "autoevaluationpipeline.h"
#include "tgt/tgt_gl.h"
#include "tgt/glcanvas.h"
#include "tgt/glcontext.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/tools/job.h"
#include "core/tools/opengljobprocessor.h"
#include "core/tools/simplejobprocessor.h"


namespace campvis {
    const std::string AutoEvaluationPipeline::loggerCat_ = "CAMPVis.core.datastructures.AutoEvaluationPipeline";

    AutoEvaluationPipeline::AutoEvaluationPipeline(DataContainer* dc) 
        : AbstractPipeline(dc)
    {
    }

    AutoEvaluationPipeline::~AutoEvaluationPipeline() {
    }

    void AutoEvaluationPipeline::init() {
        AbstractPipeline::init();

        // connect invalidation of each processor
        for (std::vector<AbstractProcessor*>::iterator it = _processors.begin(); it != _processors.end(); ++it) {
            (*it)->s_invalidated.connect(this, &AutoEvaluationPipeline::onProcessorInvalidated);
        }
    }

    void AutoEvaluationPipeline::deinit() {
        for (std::vector<AbstractProcessor*>::iterator it = _processors.begin(); it != _processors.end(); ++it) {
            (*it)->s_invalidated.disconnect(this);
        }
    
        AbstractPipeline::deinit();
    }

    void AutoEvaluationPipeline::onProcessorInvalidated(AbstractProcessor* processor) {
        if (_canvas == 0)
            return;

        tbb::concurrent_hash_map<AbstractProcessor*, bool>::const_accessor a;
        if (_isVisProcessorMap.find(a, processor)) {
            if (a->second) {
                // is VisualizationProcessor
                GLJobProc.enqueueJob(
                    _canvas, 
                    makeJobOnHeap<AutoEvaluationPipeline, AbstractProcessor*>(this, &AutoEvaluationPipeline::executeProcessorAndCheckOpenGLState, processor), 
                    OpenGLJobProcessor::SerialJob);
            }
            else {
                SimpleJobProc.enqueueJob(makeJob<AutoEvaluationPipeline, AbstractProcessor*, bool>(this, &AutoEvaluationPipeline::executeProcessor, processor, false));
            }
        }
        else {
            tgtAssert(false, "Could not find processor in processor map.");
            LWARNING("Caught invalidation of a non-registered processor!");
        }
    }

    void AutoEvaluationPipeline::addProcessor(AbstractProcessor* processor) {
        _isVisProcessorMap.insert(std::make_pair(processor, (dynamic_cast<VisualizationProcessor*>(processor) != 0)));

        PropertyCollection pc = processor->getProperties();
        for (size_t i = 0; i < pc.size(); ++i) {
            if (DataNameProperty* dnp = dynamic_cast<DataNameProperty*>(pc[i])) {
                if (dnp->getAccessInfo() == DataNameProperty::READ) {
                    tbb::spin_rw_mutex::scoped_lock lock(_pmMutex, false);
                    std::pair<PortMapType::iterator, bool> result = _portMap.insert(std::make_pair(dnp->getValue(), dnp));
                    tgtAssert(result.second, "Could not insert Property into port map!");
                    if (result.second) {
                        _iteratorMap[dnp] = result.first;
                        dnp->s_changed.connect(this, &AutoEvaluationPipeline::onDataNamePropertyChanged);
                    }
                }
            }
        }

        AbstractPipeline::addProcessor(processor);
    }

    void AutoEvaluationPipeline::onDataNamePropertyChanged(const AbstractProperty* prop) {
        DataNameProperty* dnp = const_cast<DataNameProperty*>(static_cast<const DataNameProperty*>(prop));

        // find string-iterator pair
        IteratorMapType::iterator it = _iteratorMap.find(dnp);
        if (it != _iteratorMap.end()) {
            if (dnp->getValue() != it->second->first) {
                {
                    tbb::spin_rw_mutex::scoped_lock lock(_pmMutex, true);
                    // value of the property has changed
                    _portMap.unsafe_erase(it->second);
                }

                tbb::spin_rw_mutex::scoped_lock lock(_pmMutex, false);
                std::pair<PortMapType::iterator, bool> result = _portMap.insert(std::make_pair(dnp->getValue(), dnp));
                tgtAssert(result.second, "Could not insert Property into port map!");
                if (result.second) {
                    it->second = result.first;
                }
            }
        }
        else {
            tgtAssert(false, "Could not find Property in iterator map!");
        }
    }

    void AutoEvaluationPipeline::onDataContainerDataAdded(const std::string& name, const DataHandle& dh) {
        {
            tbb::spin_rw_mutex::scoped_lock lock(_pmMutex, false);
            PortMapType::const_iterator it = _portMap.find(name);
            while(it != _portMap.end() && it->first == name) {
                it->second->s_changed(it->second);
                ++it;
            }
        }

        AbstractPipeline::onDataContainerDataAdded(name, dh);
    }

}
