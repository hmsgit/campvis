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

#ifndef METAPROPERTYWIDGET_H__
#define METAPROPERTYWIDGET_H__


#include "core/properties/metaproperty.h"
#include "application/gui/properties/abstractpropertywidget.h"
#include "application/gui/properties/propertycollectionwidget.h"
#include "application/gui/properties/propertywidgetfactory.h"

namespace campvis {
    class DataContainer;

    /**
     * Widget for a Camera.
     * For now just offering read-access.
     */
    class MetaPropertyWidget : public AbstractPropertyWidget {
        Q_OBJECT;

    public:
        /**
         * Creates a new MetaPropertyWidget for the property \a property.
         * \param   property        The property the widget shall handle
         * \param   dataContainer   DataContainer to use, must not be 0.
         * \param   parent          Parent Qt widget
         */
        MetaPropertyWidget(MetaProperty* property, DataContainer* dataContainer, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~MetaPropertyWidget();

    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty();


    private:
        PropertyCollectionWidget* _pcw;

    };

    // explicitly instantiate template, so that it gets registered also over DLL boundaries.
    template class PropertyWidgetRegistrar<MetaPropertyWidget, MetaProperty>;
}

#endif // METAPROPERTYWIDGET_H__
