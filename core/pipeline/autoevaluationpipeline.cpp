// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#include "autoevaluationpipeline.h"
#include "tgt/tgt_gl.h"
#include "tgt/glcanvas.h"
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
        if (_canvas == 0 || _enabled == false)
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
