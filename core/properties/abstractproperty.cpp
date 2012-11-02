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

#include "abstractproperty.h"

namespace campvis {

    const std::string AbstractProperty::loggerCat_ = "CAMPVis.core.datastructures.AbstractProperty";

    AbstractProperty::AbstractProperty(const std::string& name, const std::string& title, InvalidationLevel il /*= InvalidationLevel::INVALID_RESULT*/)
        : _name(name)
        , _title(title)
        , _invalidationLevel(il)
    {
        _inUse = false;
    }

    AbstractProperty::~AbstractProperty() {
    }

    const std::string& AbstractProperty::getName() {
        return _name;
    }

    const std::string& AbstractProperty::getTitle() {
        return _title;
    }

    const InvalidationLevel& AbstractProperty::getInvalidationLevel() const {
        return _invalidationLevel;
    }

    void AbstractProperty::setInvalidationLevel(const InvalidationLevel& il) {
        _invalidationLevel = il;
    }

    void AbstractProperty::addSharedProperty(AbstractProperty* prop) {
        tgtAssert(prop != 0, "Shared property must not be 0!");
        tgtAssert(typeid(this) == typeid(prop), "Shared property must be of the same type as this property.");

        tbb::spin_mutex::scoped_lock lock(_localMutex);
        _sharedProperties.insert(prop);
    }

    void AbstractProperty::removeSharedProperty(AbstractProperty* prop) {
        tgtAssert(prop != 0, "Shared property must not be 0!");
        tbb::spin_mutex::scoped_lock lock(_localMutex);
        _sharedProperties.erase(prop);
    }

    const std::set<AbstractProperty*>& AbstractProperty::getSharedProperties() const {
        return _sharedProperties;
    }

    void AbstractProperty::lock() {
        _inUse = true;
    }

    void AbstractProperty::unlock() {
        _inUse = false;
    }

    void AbstractProperty::init() {

    }

    void AbstractProperty::deinit() {

    }

}
