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

#include "propertycollectionluascriptgenerator.h"

#include "abstractpropertylua.h"
#include "propertyluafactory.h"
#include "core/datastructures/datacontainer.h"
#include "core/properties/abstractproperty.h"
#include "core/properties/propertycollection.h"

#include <QPushButton>

namespace campvis {


    PropertyCollectionLuaScriptGenerator::PropertyCollectionLuaScriptGenerator(HasPropertyCollection* propertyCollection)
        : AbstractPropertyLua(nullptr)
        , _propCollection(propertyCollection)
    {
        _propCollection = propertyCollection;

        // create widgets for the new PropertyCollection
        if (propertyCollection != 0) {
            for (std::vector<AbstractProperty*>::const_iterator it = propertyCollection->getProperties().begin(); it != propertyCollection->getProperties().end(); ++it) {
                addProperty(*it);
            } 
        }
    }

    PropertyCollectionLuaScriptGenerator::~PropertyCollectionLuaScriptGenerator() {
    }

    void PropertyCollectionLuaScriptGenerator::updatePropCollection(HasPropertyCollection* propertyCollection, DataContainer* dc) {
        _propCollection = propertyCollection;

        // create lua script for the new PropertyCollection
        if (propertyCollection != 0) {
            _luaMap.clear();
            for (std::vector<AbstractProperty*>::const_iterator it = propertyCollection->getProperties().begin(); it != propertyCollection->getProperties().end(); ++it) {
                addProperty(*it);
            }
        }
    }

    std::string PropertyCollectionLuaScriptGenerator::getLuaScript(const std::string& propNamePrefix, const std::string& luaProc) {
        std::string ret = "";
        for (std::map<AbstractProperty*, AbstractPropertyLua*>::iterator it = _luaMap.begin(); it != _luaMap.end(); ++it) {
            ret +=  it->second->getLuaScript(propNamePrefix, luaProc) + "\n";
        }
        return ret;
    }

    void PropertyCollectionLuaScriptGenerator::addProperty(AbstractProperty* prop) {
        AbstractPropertyLua* propWidget = PropertyLuaFactory::getRef().createPropertyLua(prop);
        if (propWidget == 0) {
            return;
        }
        _luaMap.insert(std::make_pair(prop, propWidget));
    }

}