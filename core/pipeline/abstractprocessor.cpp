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

#include <tbb/compat/thread>
#include "tgt/assert.h"
#include "abstractprocessor.h"
#include "core/properties/abstractproperty.h"

namespace campvis {

    const std::string AbstractProcessor::loggerCat_ = "CAMPVis.core.datastructures.Processor";


    AbstractProcessor::AbstractProcessor()
        : HasPropertyCollection()
    {
        _enabled = true;
        _clockExecutionTime = false;
        _locked = 0;
        _level = VALID;
    }

    AbstractProcessor::~AbstractProcessor() {

    }

    void AbstractProcessor::init() {
        initAllProperties();
    }

    void AbstractProcessor::deinit() {
        deinitAllProperties();
    }


    void AbstractProcessor::lockProcessor() {
        while (_locked.compare_and_swap(true, false) == true) // TODO: busy waiting us fu**ing ugly...
            std::this_thread::yield();
        _locked = true;
        lockAllProperties();
    }

    void AbstractProcessor::unlockProcessor() {
        tgtAssert(_locked == true, "Called AbstractProcessor::unlockProcessor() on unlocked processor!");
        unlockAllProperties();
        int summed = VALID;
        int il;
        while (_queuedInvalidations.try_pop(il)) {
            summed |= il;
        }
        _locked = false;

        if (summed != VALID)
            invalidate(summed);
    }

    bool AbstractProcessor::isLocked() {
        return _locked != 0;
    }

    void AbstractProcessor::onPropertyChanged(const AbstractProperty* prop) {
        HasPropertyCollection::onPropertyChanged(prop);
        invalidate(prop->getInvalidationLevel());
    }

    bool AbstractProcessor::getEnabled() const {
        return _enabled;
    }

    void AbstractProcessor::setEnabled(bool enabled) {
        _enabled = enabled;
    }

    void AbstractProcessor::invalidate(int level) {
        if (_locked) {
            // TODO: this is not 100% thread-safe - an invalidation might slip through if the processor is unlocked during invalidation
            _queuedInvalidations.push(level);
        }
        else {
            int tmp;
            do {
                tmp = _level;
            } while (_level.compare_and_swap(tmp | level, tmp) != tmp);
            s_invalidated(this);
        }
    }

    void AbstractProcessor::validate(int level) {
        int tmp;
        do {
            tmp = _level;
        } while (_level.compare_and_swap(tmp & (~level), tmp) != tmp);
        s_validated(this);
    }

    bool AbstractProcessor::getClockExecutionTime() const {
        return _clockExecutionTime;
    }

    void AbstractProcessor::setClockExecutionTime(bool value) {
        _clockExecutionTime = value;
    }

    void AbstractProcessor::updateProperties(DataContainer& dc) {
        validate(INVALID_PROPERTIES);
    }


}
