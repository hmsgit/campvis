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

#include "abstractprocessor.h"
#include "cgt/assert.h"
#include "core/properties/abstractproperty.h"

#include <ext/threading.h>

namespace campvis {

    const std::string AbstractProcessor::loggerCat_ = "CAMPVis.core.datastructures.Processor";


    AbstractProcessor::AbstractProcessor()
        : HasPropertyCollection()
    {
        _enabled = true;
        _clockExecutionTime = false;
        _ignorePropertyChanges = 0;
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
        // hopefully, it does not take too long.
        while (_locked.compare_and_swap(true, false) == true)
            std::this_thread::yield();
        _locked = true;
        lockAllProperties();
    }

    void AbstractProcessor::unlockProcessor() {
        cgtAssert(_locked == true, "Called AbstractProcessor::unlockProcessor() on unlocked processor!");
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
        if (_ignorePropertyChanges == 0) {
            tbb::spin_rw_mutex::scoped_lock lock(_mtxInvalidationMap, false);
            auto it = _invalidationMap.find(prop);
            if (it != _invalidationMap.end())
                invalidate(it->second);
            else
                LDEBUG("Caught an property changed signal that was not registered with an invalidation level. Did you forget to call addProperty()?");
        }
    }

    bool AbstractProcessor::getEnabled() const {
        return _enabled;
    }

    void AbstractProcessor::setEnabled(bool enabled) {
        _enabled = enabled;
    }

    void AbstractProcessor::invalidate(int level) {
        if (level == 0)
            return;

        if (_locked) {
            // TODO: this is not 100% thread-safe - an invalidation might slip through if the processor is unlocked during invalidation
            _queuedInvalidations.push(level);
        }
        else {
            int tmp;
            do {
                tmp = _level;
            } while (_level.compare_and_swap(tmp | level, tmp) != tmp);
            s_invalidated.emitSignal(this);
        }
    }

    void AbstractProcessor::validate(int level) {
        int tmp;
        do {
            tmp = _level;
        } while (_level.compare_and_swap(tmp & (~level), tmp) != tmp);
        s_validated.emitSignal(this);
    }

    bool AbstractProcessor::getClockExecutionTime() const {
        return _clockExecutionTime;
    }

    void AbstractProcessor::setClockExecutionTime(bool value) {
        _clockExecutionTime = value;
    }


    void AbstractProcessor::process(DataContainer& data) {
        if (hasInvalidShader()) {
            updateShader();
            validate(INVALID_SHADER);
        }
        if (hasInvalidProperties()) {
            updateProperties(data);
            validate(INVALID_PROPERTIES);
        }

        // use a scoped lock for exception safety
        AbstractProcessor::ScopedLock lock(this);
        cgtAssert(_locked == true, "Processor not locked, this should not happen!");

        if (hasInvalidResult()) {
            updateResult(data);
            validate(INVALID_RESULT);
        }
    }

    void AbstractProcessor::updateShader() {
        LDEBUG("Called non-overriden updateShader() in " << getName() << ". Did you forget to override your method?");
    }

    void AbstractProcessor::updateProperties(DataContainer& dc) {
        LDEBUG("Called non-overriden updateProperties() in " << getName() << ". Did you forget to override your method?");
    }

    void AbstractProcessor::addProperty(AbstractProperty& prop) {
        this->addProperty(prop, INVALID_RESULT);
    }

    void AbstractProcessor::addProperty(AbstractProperty& prop, int invalidationLevel) {
        HasPropertyCollection::addProperty(prop);
        setPropertyInvalidationLevel(prop, invalidationLevel);
    }

    void AbstractProcessor::setPropertyInvalidationLevel(AbstractProperty& prop, int invalidationLevel) {
        tbb::spin_rw_mutex::scoped_lock lock(_mtxInvalidationMap, true);
        _invalidationMap[&prop] = invalidationLevel;
    }
    void AbstractProcessor::ignorePropertyChanges() {
        ++_ignorePropertyChanges;
    }

    void AbstractProcessor::observePropertyChanges() {
        --_ignorePropertyChanges;
    }
}
