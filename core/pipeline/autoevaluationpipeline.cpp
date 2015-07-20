// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
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

#include "cgt/cgt_gl.h"
#include "cgt/glcanvas.h"

#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/metaproperty.h"

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
    }

    void AutoEvaluationPipeline::deinit() {
        for (std::vector<AbstractProcessor*>::iterator it = _processors.begin(); it != _processors.end(); ++it) {
            (*it)->s_invalidated.disconnect(this);
        }

        AbstractPipeline::deinit();
    }

    void AutoEvaluationPipeline::onProcessorInvalidated(AbstractProcessor* processor) {
        if (_canvas == nullptr || getEnabled() == false)
            return;

        setPipelineDirty();
    }

    void AutoEvaluationPipeline::addProcessor(AbstractProcessor* processor) {
        _isVisProcessorMap.insert(std::make_pair(processor, (dynamic_cast<VisualizationProcessor*>(processor) != 0)));
        findDataNamePropertiesAndAddToPortMap(processor);

        AbstractPipeline::addProcessor(processor);
        processor->s_invalidated.connect(this, &AutoEvaluationPipeline::onProcessorInvalidated);
    }

    void AutoEvaluationPipeline::executePipeline() {
        // execute each processor once
        // (AbstractProcessor::process() takes care of executing only invalid processors)
        for (size_t i = 0; i < _processors.size(); ++i) {
            executeProcessorAndCheckOpenGLState(_processors[i]);
        }
    }

    void AutoEvaluationPipeline::onDataNamePropertyChanged(const AbstractProperty* prop) {
        // static_cast is safe since this slot only get called for DataNameProperties
        // const_cast is not beautiful but safe here as well
        DataNameProperty* dnp = const_cast<DataNameProperty*>(static_cast<const DataNameProperty*>(prop));

        // find string-iterator pair for the given property
        IteratorMapType::iterator it = _iteratorMap.find(dnp);
        if (it != _iteratorMap.end()) {
            // check whether the value of the DataNameProperty differs from the one in our port map
            // i.e.: We need to update the port map
            if (dnp->getValue() != it->second->first) {
                {
                    // acquire a write-lock since we erase the old value from our port map
                    tbb::spin_rw_mutex::scoped_lock lock(_pmMutex, true);
                    _portMap.unsafe_erase(it->second);
                }

                // acquire read-lock since we add the new value to the port map
                tbb::spin_rw_mutex::scoped_lock lock(_pmMutex, false);

                // insert new value into port map and update the reference in the iterator map
                std::pair<PortMapType::iterator, bool> result = _portMap.insert(std::make_pair(dnp->getValue(), dnp));
                if (result.second) {
                    it->second = result.first;
                }

                // sanity check, if this assertion fails, we have a problem...
                cgtAssert(result.second, "Could not insert Property into port map!");
            }
        }
        else {
            // this should not happen, otherwise we did something wrong before.
            cgtAssert(false, "Could not find Property in iterator map!");
        }
    }

    namespace {
        /// Local helper function to recursively emit the s_changed signal for the given
        /// properties and all of its shared properties.
        void recursiveEmitSignal(DataNameProperty* p) {
            std::set<AbstractProperty*> v = p->getSharedProperties();
            for (auto it = v.begin(); it != v.end(); ++it)
                recursiveEmitSignal(static_cast<DataNameProperty*>(*it));
            p->s_changed.emitSignal(p);
        }
    }

    void AutoEvaluationPipeline::onDataContainerDataAdded(std::string name, DataHandle dh) {
        {
            AbstractPipeline::onDataContainerDataAdded(name, dh);

            // acquire read lock
            tbb::spin_rw_mutex::scoped_lock lock(_pmMutex, false);

            // find all DataNameProperties in the port map that have name as current value
            PortMapType::const_iterator it = _portMap.find(name);
            while (it != _portMap.end() && it->first == name) {
                // invalidate those properties by emitting changed signal
                recursiveEmitSignal(it->second);
                ++it;
            }
        }
    }

    void AutoEvaluationPipeline::findDataNamePropertiesAndAddToPortMap(const HasPropertyCollection* hpc) {
        const PropertyCollection& pc = hpc->getProperties();

        // const_cast okay, since we're just connecting to signals
        const_cast<HasPropertyCollection*>(hpc)->s_propertyAdded.connect(this, &AutoEvaluationPipeline::onPropertyCollectionPropertyAdded);
        const_cast<HasPropertyCollection*>(hpc)->s_propertyRemoved.connect(this, &AutoEvaluationPipeline::onPropertyCollectionPropertyRemoved);

        // traverse property collection
        for (size_t i = 0; i < pc.size(); ++i) {
            if (DataNameProperty* dnp = dynamic_cast<DataNameProperty*>(pc[i])) {
                // if DataNameProperty, add to port map and register to changed signal
                if (dnp->getAccessInfo() == DataNameProperty::READ) {
                    tbb::spin_rw_mutex::scoped_lock lock(_pmMutex, false);
                    std::pair<PortMapType::iterator, bool> result = _portMap.insert(std::make_pair(dnp->getValue(), dnp));
                    cgtAssert(result.second, "Could not insert Property into port map!");
                    if (result.second) {
                        _iteratorMap[dnp] = result.first;
                        dnp->s_changed.connect(this, &AutoEvaluationPipeline::onDataNamePropertyChanged);
                    }
                }
            }
            else if (MetaProperty* mp = dynamic_cast<MetaProperty*>(pc[i])) {
                // if MetaProperty, recursively check its PropertyCollection
                findDataNamePropertiesAndAddToPortMap(mp);
            }
        }
    }

    void AutoEvaluationPipeline::onPropertyCollectionPropertyAdded(AbstractProperty* property) {
        // check whether the incoming property is of the correct type (we only care about DataNameProperties)
        if (DataNameProperty* dnp = dynamic_cast<DataNameProperty*>(property)) {
            if (dnp->getAccessInfo() == DataNameProperty::READ) {
                // check whether this property is already present in the port map
                IteratorMapType::iterator it = _iteratorMap.find(dnp);
                if (it == _iteratorMap.end()) {
                    // add to port map and register to changed signal
                    tbb::spin_rw_mutex::scoped_lock lock(_pmMutex, false);
                    std::pair<PortMapType::iterator, bool> result = _portMap.insert(std::make_pair(dnp->getValue(), dnp));
                    cgtAssert(result.second, "Could not insert Property into port map!");
                    if (result.second) {
                        _iteratorMap[dnp] = result.first;
                        dnp->s_changed.connect(this, &AutoEvaluationPipeline::onDataNamePropertyChanged);
                    }
                }
                else {
                    // this should not happen, otherwise we did something wrong before.
                    cgtAssert(false, "This property is already in iterator map!");
                }
            }
        }
    }

    void AutoEvaluationPipeline::onPropertyCollectionPropertyRemoved(AbstractProperty* property) {
        // check whether the incoming property is of the correct type (we only care about DataNameProperties)
        if (DataNameProperty* dnp = dynamic_cast<DataNameProperty*>(property)) {
            if (dnp->getAccessInfo() == DataNameProperty::READ) {
                // find string-iterator pair for the given property
                IteratorMapType::iterator it = _iteratorMap.find(dnp);
                if (it != _iteratorMap.end()) {
                    // remove from port map and deregister from changed signal
                    dnp->s_changed.disconnect(this);

                    // acquire a write-lock since we erase the old value from our port map
                    tbb::spin_rw_mutex::scoped_lock lock(_pmMutex, true);

                    // remove the property from the port map
                    _portMap.unsafe_erase(it->second);
                    _iteratorMap.unsafe_erase(it);
                }
                else {
                    // this should not happen, otherwise we did something wrong before.
                    cgtAssert(false, "Could not find Property in iterator map!");
                }
            }
        }
    }

}
