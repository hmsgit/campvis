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

#include "abstractproperty.h"
#include "cgt/logmanager.h"

namespace campvis {

    const std::string AbstractProperty::loggerCat_ = "CAMPVis.core.datastructures.AbstractProperty";

    AbstractProperty::AbstractProperty(const std::string& name, const std::string& title)
        : _name(name)
        , _title(title)
    {
        _isVisible = true;
        _inUse = 0;
    }

    AbstractProperty::~AbstractProperty() {

    }

    const std::string& AbstractProperty::getName() const {
        return _name;
    }

    const std::string& AbstractProperty::getTitle() const {
        return _title;
    }

    void AbstractProperty::addSharedProperty(AbstractProperty* prop) {
        cgtAssert(prop != nullptr, "Shared property must not be 0!");
        cgtAssert(prop != this, "Shared property must not be this!");

        if (typeid(this) == typeid(prop)) {
            tbb::spin_mutex::scoped_lock lock(_localMutex);
            _sharedProperties.insert(prop);
        }
        else {
            LERROR("Could not add " << prop->getName() << " as shared property of " << getName() << " since they are not of the same type.");
        }
    }

    void AbstractProperty::removeSharedProperty(AbstractProperty* prop) {
        cgtAssert(prop != 0, "Shared property must not be 0!");
        tbb::spin_mutex::scoped_lock lock(_localMutex);
        _sharedProperties.erase(prop);
    }

    const std::set<AbstractProperty*>& AbstractProperty::getSharedProperties() const {
        return _sharedProperties;
    }

    void AbstractProperty::lock() {
        ++_inUse;
    }

    void AbstractProperty::unlock() {
        --_inUse;
    }

    void AbstractProperty::init() {

    }

    void AbstractProperty::deinit() {

    }

    bool AbstractProperty::isVisible() const {
        return _isVisible;
    }

    void AbstractProperty::setVisible(bool isVisible) {
        if (_isVisible != isVisible) {
            _isVisible = isVisible;
            s_visibilityChanged.emitSignal(this);
        }
    }

}
