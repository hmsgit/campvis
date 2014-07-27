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

#include "propertywidgetfactory.h"

#include <functional>

#include "core/properties/abstractproperty.h"

namespace campvis {

    tbb::atomic<PropertyWidgetFactory*> PropertyWidgetFactory::_singleton;

    PropertyWidgetFactory& PropertyWidgetFactory::getRef() {
        if (_singleton == 0) {
            std::cout << "creating PropertyWidgetFactory...\n";
            PropertyWidgetFactory* tmp = new PropertyWidgetFactory();
            if (_singleton.compare_and_swap(tmp, 0) != 0) {
                delete tmp;
            }
        }

        return *_singleton;
    }

    void PropertyWidgetFactory::deinit() {
        delete _singleton;
        _singleton = nullptr;
    }

    size_t PropertyWidgetFactory::registerPropertyWidget(const std::type_info& type, PropertyWidgetCreateFunctionPointer ptr, FallbackPropertyWidgetCreateFunctionPointer fallbackPtr, int priority) {
        tbb::spin_mutex::scoped_lock lock(_mutex);
        std::type_index typeIndex(type);

        if (ptr != nullptr) {
            PropertyWidgetMapType::iterator it = _propertyWidgetMap.lower_bound(typeIndex);
            if (it == _propertyWidgetMap.end() || it->first != typeIndex) {
                _propertyWidgetMap.insert(it, std::make_pair(typeIndex, ptr));
            }
            else {
                tgtAssert(false, "Double-registered a property widget for the same type.");
            }
        }
        if (fallbackPtr != nullptr) {
            _fallbackCreatorMap.insert(std::make_pair(-priority, fallbackPtr));
        }

        return _propertyWidgetMap.size() + _fallbackCreatorMap.size();
    }

    AbstractPropertyWidget* PropertyWidgetFactory::createWidget(AbstractProperty* prop, DataContainer* dc /*= 0*/, QWidget* parent /*= 0*/) {
        // look if we find a direct a direct match
        PropertyWidgetMapType::iterator it = _propertyWidgetMap.find(std::type_index(typeid(*prop)));
        if (it != _propertyWidgetMap.end()) {
            return it->second(prop, dc, parent);
        }

        // otherwise we have to do this kind of slow search
        for (std::multimap<int, FallbackPropertyWidgetCreateFunctionPointer>::iterator it = _fallbackCreatorMap.begin(); it != _fallbackCreatorMap.end(); ++it) {
            // let each registered widget try to create a widget for the property
            AbstractPropertyWidget* toReturn = it->second(prop, dc, parent);
            if (toReturn != nullptr)
                return toReturn;
        }

        return nullptr;
    }

}