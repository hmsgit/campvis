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

#include "propertycollection.h"

#include "core/properties/abstractproperty.h"
#include "core/properties/metaproperty.h"
#include "core/tools/stringutils.h"

namespace campvis {
    HasPropertyCollection::HasPropertyCollection() {
    }

    HasPropertyCollection::~HasPropertyCollection() {
    }

    void HasPropertyCollection::addProperty(AbstractProperty& prop) {
        PropertyCollection::iterator it = findProperty(prop.getName());
        if (it != _properties.end()) {
            (*it)->s_changed.disconnect(this);
            s_propertyRemoved.emitSignal(*it);
            *it = &prop;
        }
        else {
            _properties.push_back(&prop);
        }
        prop.s_changed.connect(this, &HasPropertyCollection::onPropertyChanged);
        s_propertyAdded.emitSignal(&prop);
    }

    void HasPropertyCollection::removeProperty(AbstractProperty& prop) {
        PropertyCollection::iterator it = findProperty(prop.getName());
        if (it != _properties.end()) {
            (*it)->s_changed.disconnect(this);
            _properties.erase(it);
            s_propertyRemoved.emitSignal(&prop);
        }
    }

    AbstractProperty* HasPropertyCollection::getProperty(const std::string& name) const {
        PropertyCollection::const_iterator it = findProperty(name);
        if (it != _properties.end())
            return *it;

        return 0;
    }

    AbstractProperty* HasPropertyCollection::getNestedProperty(const std::string& name) const {
        // try to find nested property (use :: as delimiter)
        std::vector<std::string> levels = StringUtils::split(name, "::");
        AbstractProperty* toReturn = getProperty(levels[0]);
        size_t currentLevel = 1;
        while (toReturn != 0 && currentLevel < levels.size()) {
            if (MetaProperty* tester = dynamic_cast<MetaProperty*>(toReturn)) {
            	toReturn = tester->getProperty(levels[currentLevel]);
                ++currentLevel;
            }
            else {
                toReturn = 0;
            }
        }

        return toReturn;
    }

    const PropertyCollection& HasPropertyCollection::getProperties() const {
        return _properties;
    }

    PropertyCollection& HasPropertyCollection::getProperties() {
        return _properties;
    }

    PropertyCollection::iterator HasPropertyCollection::findProperty(const std::string& name) {
        // using std::find would be more elegant, but also need more LOC...
        for (PropertyCollection::iterator it = _properties.begin(); it != _properties.end(); ++it) {
            if ((*it)->getName() == name)
                return it;
        }
        return _properties.end();
    }

    PropertyCollection::const_iterator HasPropertyCollection::findProperty(const std::string& name) const {
        // using std::find would be more elegant, but also need more LOC...
        for (PropertyCollection::const_iterator it = _properties.begin(); it != _properties.end(); ++it) {
            if ((*it)->getName() == name)
                return it;
        }
        return _properties.end();
    }

    void HasPropertyCollection::lockAllProperties() {
        for (PropertyCollection::iterator it = _properties.begin(); it != _properties.end(); ++it) {
            (*it)->lock();
        }
    }

    void HasPropertyCollection::unlockAllProperties() {
        for (PropertyCollection::iterator it = _properties.begin(); it != _properties.end(); ++it) {
            (*it)->unlock();
        }
    }

    void HasPropertyCollection::initAllProperties() {
        for (PropertyCollection::iterator it = _properties.begin(); it != _properties.end(); ++it) {
            (*it)->init();
        }
    }

    void HasPropertyCollection::deinitAllProperties() {
        for (PropertyCollection::iterator it = _properties.begin(); it != _properties.end(); ++it) {
            (*it)->deinit();
            (*it)->s_changed.disconnect(this);
        }
    }

    void HasPropertyCollection::onPropertyChanged(const AbstractProperty* /*prop*/) {
        // nothing to do here, method is just provided as convenience for child classes.
    }

}
