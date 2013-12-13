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

#ifndef PROPERTYCOLLECTIONWIDGET_H__
#define PROPERTYCOLLECTIONWIDGET_H__

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
    class PropertyCollectionWidget : public QWidget, public sigslot::has_slots<> {
        Q_OBJECT;

    public:
        /**
         * Creates a new PropertyCollectionWidget.
         * \param   parent  Parent widget, may be 0.
         */
        PropertyCollectionWidget(QWidget* parent = 0);

        /**
         * Destructor.
         */
        ~PropertyCollectionWidget();

    public slots:
        /**
         * Updates the property collection this widget works on.
         * \param   propertyCollection  New HasPropertyCollection instance for this widget, may be 0.
         */
        void updatePropCollection(HasPropertyCollection* propertyCollection, DataContainer* dc);

        /**
         * Slot to be called when one of the propertie's visibility has changed.
         * \param prop  Property that emitted the signal
         */
        void onPropertyVisibilityChanged(const AbstractProperty* prop);

    private:
        /**
         * Sets up this widget
         */
        void setupWidget();

        /**
         * Clears the _widgetMap and destroys all widgets inside.
         */
        void clearWidgetMap();

        HasPropertyCollection* _propCollection;    ///< The HasPropertyCollection instance this widget is currently working on.
        QVBoxLayout* _layout;
        std::map<AbstractProperty*, QWidget*> _widgetMap;
        //QList<QWidget*> _widgetList;
    };
}

#endif // PROPERTYCOLLECTIONWIDGET_H__
