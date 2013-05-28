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

#include "tbb/compat/thread"
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


}
