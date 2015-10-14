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

#ifndef BUTTONPROPERTYWIDGET_H__
#define BUTTONPROPERTYWIDGET_H__

#include "application/applicationapi.h"
#include "application/gui/properties/abstractpropertywidget.h"
#include "application/gui/properties/propertywidgetfactory.h"
#include "core/properties/buttonproperty.h"

class QPushButton;

namespace campvis {
    /**
     * Widget for a Camera.
     * For now just offering read-access.
     */
    class CAMPVIS_APPLICATION_API ButtonPropertyWidget : public AbstractPropertyWidget {
        Q_OBJECT;

    public:
        /**
         * Creates a new ButtonPropertyWidget for the property \a property.
         * \param   property    The property the widget shall handle
         * \param   parent      Parent Qt widget
         */
        ButtonPropertyWidget(ButtonProperty* property, DataContainer* dataContainer = nullptr, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~ButtonPropertyWidget();

    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty();

    protected slots:
        void onButtonClicked(bool);

    private:
        QPushButton* _button;

    };

    // explicitly instantiate template, so that it gets registered also over DLL boundaries.
    template class PropertyWidgetRegistrar<ButtonPropertyWidget, ButtonProperty>;
}

#endif // BUTTONPROPERTYWIDGET_H__
