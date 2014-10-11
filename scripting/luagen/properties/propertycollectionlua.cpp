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

#include "propertycollectionlua.h"

#include "abstractpropertylua.h"
#include "propertyluafactory.h"
#include "core/datastructures/datacontainer.h"
#include "core/properties/abstractproperty.h"
#include "core/properties/propertycollection.h"

#include <QPushButton>

namespace campvis {


    PropertyCollectionLua::PropertyCollectionLua()
        : _propCollection(0)
        , _dataContainer(0)
    {
        setupLua();
    }

    PropertyCollectionLua::~PropertyCollectionLua() {
        clearLuaMap();
    }

    void PropertyCollectionLua::updatePropCollection(HasPropertyCollection* propertyCollection, DataContainer* dc) {
        // remove and delete all widgets of the previous PropertyCollection
        clearLuaMap();
        
        _propCollection = propertyCollection;
        _dataContainer = dc;

        // create widgets for the new PropertyCollection
        if (propertyCollection != 0) {
            for (std::vector<AbstractProperty*>::const_iterator it = propertyCollection->getProperties().begin(); it != propertyCollection->getProperties().end(); ++it) {
                addProperty(*it);
            } 

            //propertyCollection->s_propertyAdded.connect(this, &PropertyCollectionLua::onPropCollectionPropAdded);
            //propertyCollection->s_propertyRemoved.connect(this, &PropertyCollectionLua::onPropCollectionPropRemoved);
        }
    }

    void PropertyCollectionLua::setupLua() {
    
    }

    void PropertyCollectionLua::clearLuaMap() {
        for (std::map<AbstractProperty*, AbstractPropertyLua*>::iterator it = _luaMap.begin(); it != _luaMap.end(); ++it) {
            removeProperty(it);
        }

        _luaMap.clear();

        if (_propCollection != 0) {
            //_propCollection->s_propertyAdded.disconnect(this);
            //_propCollection->s_propertyRemoved.disconnect(this);
        }
    }

    void PropertyCollectionLua::onPropertyVisibilityChanged(const AbstractProperty* prop) {
        // const_cast does not harm anything.
        std::map<AbstractProperty*, AbstractPropertyLua*>::iterator item = _luaMap.find(const_cast<AbstractProperty*>(prop));
        //if (item != _luaMap.end())
            //emit s_luaVisibilityChanged(item->second, item->first->isVisible());
    }

    void PropertyCollectionLua::onPropCollectionPropAdded(AbstractProperty* prop) {
        //emit propertyAdded(prop);
    }

    void PropertyCollectionLua::onPropCollectionPropRemoved(AbstractProperty* prop) {
        std::map<AbstractProperty*, AbstractPropertyLua*>::iterator it = _luaMap.find(prop);
        //if (it != _luaMap.end())
            //emit propertyRemoved(it);
    }

    void PropertyCollectionLua::addProperty(AbstractProperty* prop) {
        AbstractPropertyLua* propWidget = PropertyLuaFactory::getRef().createPropertyLua(prop, _dataContainer);
        if (propWidget == 0)
            return;
        //    propWidget = new AbstractPropertyLua(QString::fromStdString(prop->getTitle()));

        _luaMap.insert(std::make_pair(prop, propWidget));
        propWidget->getLuaScript();

        prop->s_visibilityChanged.connect(this, &PropertyCollectionLua::onPropertyVisibilityChanged);
        prop->s_visibilityChanged.emitSignal(prop);
    }

    void PropertyCollectionLua::removeProperty(std::map<AbstractProperty*, AbstractPropertyLua*>::iterator it) {
        it->first->s_visibilityChanged.disconnect(this);
        delete it->second;
    }

}