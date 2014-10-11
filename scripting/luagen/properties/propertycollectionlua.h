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

#ifndef PROPERTYCOLLECTIONLUA_H__
#define PROPERTYCOLLECTIONLUA_H__

#include "propertycollectionlua.h"
#include "abstractpropertylua.h"
#include "sigslot/sigslot.h"
#include <QList>
#include <QVBoxLayout>
#include <QWidget>
#include <map>

namespace campvis {
    class AbstractProperty;
    class DataContainer;
    class HasPropertyCollection;

    /**
     * Main Window for the CAMPVis application.
     * Wraps a nice Qt GUI around the TumVisApplication instance given during creation.
     */
    class PropertyCollectionLua : public sigslot::has_slots {
    public:
        /**
         * Creates a new PropertyCollectionLua.
         * \param   parent  Parent widget, may be 0.
         */
        PropertyCollectionLua();

        /**
         * Destructor.
         */
        ~PropertyCollectionLua();

    public slots:
        /**
         * Updates the property collection this widget works on.
         * \param   propertyCollection  New HasPropertyCollection instance for this widget, may be 0.
         */
        void updatePropCollection(HasPropertyCollection* propertyCollection, DataContainer* dc);

        /**
         * Slot to be called when one of the properties' visibility has changed.
         * \param prop  Property that emitted the signal
         */
        void onPropertyVisibilityChanged(const AbstractProperty* prop);
        
    protected slots:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        //virtual void onLuaVisibilityChanged(AbstractPropertyLua* propLua, bool visibility);

        /**
         * Creates the property widget for \a prop, connects all necessary signals, etc.
         * \param   prop    Property to add
         */
        void addProperty(AbstractProperty* prop);

        /**
         * Removes the property widget for \a prop, disconnects all necessary signals, etc.
         * \param   prop    Iterator to widget map for the property to remove.
         */
        void removeProperty(std::map<AbstractProperty*, AbstractPropertyLua*>::iterator prop);

    signals:
        //void s_luaVisibilityChanged(AbstractPropertyLua* widget, bool visibility);

        //void propertyAdded(AbstractProperty* prop);
        //void propertyRemoved(std::map<AbstractProperty*, AbstractPropertyLua*>::iterator prop);

    private:
        /**
         * Sets up this widget
         */
        void setupLua();

        /**
         * Clears the _luaMap and destroys all widgets inside.
         */
        void clearLuaMap();

        /**
         * Slot called from PropertyCollection a property was added.
         */
        void onPropCollectionPropAdded(AbstractProperty* prop);

        /**
         * Slot called from PropertyCollection a property was removed.
         */
        void onPropCollectionPropRemoved(AbstractProperty* prop);

        HasPropertyCollection* _propCollection; ///< The HasPropertyCollection instance this widget is currently working on.
        DataContainer* _dataContainer;          ///< The DataContainer the properties shall work on

        std::map<AbstractProperty*, AbstractPropertyLua*> _luaMap;
    };
}


#endif // PROPERTYCOLLECTIONLUA_H__
