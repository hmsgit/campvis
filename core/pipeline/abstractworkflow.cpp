
// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

#include "abstractworkflow.h"

#include "cgt/assert.h"

namespace campvis {
    const std::string AbstractWorkflow::loggerCat_ = "CAMPVis.core.pipeline.AbstractWorkflow";

    AbstractWorkflow::AbstractWorkflow() 
        : _currentStage(0)
    {

    }

    AbstractWorkflow::~AbstractWorkflow() {
        // delete only stages, not the DataContainers or Pipelines as we don't own them
        for (auto it = _stages.begin(); it != _stages.end(); ++it)
            delete it->second;
    }

    void AbstractWorkflow::init() {
        cgtAssert(! _stages.empty(), "Tried to initialize a workflow without any stages.");

        // default initialize the workflow with the stage with the lowest ID
        if (_stages.find(_currentStage) == _stages.end())
            _currentStage = _stages.begin()->first;
    }

    void AbstractWorkflow::deinit() {

    }

    bool AbstractWorkflow::isStageAvailable(int stage) const {
        // the default implementation always returns true
        return true;
    }

    int AbstractWorkflow::getCurrentStageId() const {
        return _currentStage;
    }

    const AbstractWorkflow::Stage& AbstractWorkflow::getCurrentStage() const {
        cgtAssert(_stages.find(_currentStage) != _stages.end(), "Could not find current stage in workflow stage map. This must not happen!");

        // use at() since there is no const overload of operator[]...
        const Stage* toReturn = _stages.at(_currentStage);
        return *toReturn;
    }

    void AbstractWorkflow::setCurrentStage(int stage) {
        int oldStage = _currentStage;
        _currentStage = stage;

        // update visible properties
        clearProperties();
        Stage* s = _stages[_currentStage];
        for (auto it = s->_visibleProperties.begin(); it != s->_visibleProperties.end(); ++it) {
            addProperty(**it);
        }

        // notify the outer world
        s_stageChanged.emitSignal(oldStage, _currentStage);
    }

    void AbstractWorkflow::addStage(int id, const std::string& title, const std::vector<AbstractProperty*>& visibleProperties /*= std::vector<AbstractProperty*>()*/) {
        cgtAssert(_stages.find(id) == _stages.end(), "Tried two stages with the same ID.");

        Stage* s = new Stage();
        s->_id = id;
        s->_title = title;
        s->_visibleProperties = visibleProperties;

        _stages[id] = s;
    }

    void AbstractWorkflow::addStageTransition(int from, int to) {
        cgtAssert(_stages.find(from) == _stages.end(), "Tried to register a stage transition with an ivalid stage ID.");
        cgtAssert(_stages.find(to) == _stages.end(), "Tried to register a stage transition with an ivalid stage ID.");

        _stages[from]->_possibleTransitions.push_back(_stages[to]);
    }
    
}
