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

#ifndef COLORPROPERTYWIDGET_H__
#define COLORPROPERTYWIDGET_H__

#include "application/gui/adjusterwidgets/doubleadjusterwidget.h"
#include "application/gui/properties/abstractpropertywidget.h"
#include "application/gui/properties/propertywidgetfactory.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/colorproperty.h"

#include "application/gui/qtcolortools.h"


namespace campvis {
    /**
     * Widget for a ColorProperty
     */
    class ColorPropertyWidget : public AbstractPropertyWidget {
        Q_OBJECT

    public:
        /**
         * Creates a new FloatPropertyWidget for the property \a property.
         * \param   property    The property the widget shall handle
         * \param   dataContainer   DataContainer to use (optional), defaults to nullptr.
         * \param   parent      Parent Qt widget
         */
        ColorPropertyWidget(ColorProperty* property, DataContainer* dataContainer, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~ColorPropertyWidget();

    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty();

    private slots:
        /// Slot getting called when the adjuster's value changes
        void onColorChanged(const QColor&);

    private:
        /// Slot getting called when the property's min or max value has changed, so that the widget can be updated.
        virtual void onPropertyMinMaxChanged(const AbstractProperty* property);

        //QLabel *_lblColorPicker;
        ColorPickerWidget * _colorPicker;        ///< color picker widget allowing to change color

    }; 

// ================================================================================================

    // explicitly instantiate template, so that it gets registered also over DLL boundaries.   
    template class PropertyWidgetRegistrar<ColorPropertyWidget, ColorProperty>;

}

#endif // COLORPROPERTYWIDGET_H__
