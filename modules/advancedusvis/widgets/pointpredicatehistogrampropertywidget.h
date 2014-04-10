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

#ifndef POINTPREDICATEHISTOGRAMPROPERTYWIDGET_H__
#define POINTPREDICATEHISTOGRAMPROPERTYWIDGET_H__


#include "application/gui/properties/abstractpropertywidget.h"
#include "application/gui/properties/propertycollectionwidget.h"
#include "application/gui/properties/propertywidgetfactory.h"

#include "modules/advancedusvis/widgets/pointpredicaterenderarea.h"
#include "modules/advancedusvis/properties/pointpredicatehistogramproperty.h"

#include <QComboBox>
#include <QLabel>
#include <QListWidget>
#include <QPushButton>
#include <QSlider>

namespace campvis {
    /**
     * Widget for a PointPredicateHistogramProperty.
     * Offers beautiful rendering of the histogram and all controls needed for configuration.
     */
    class PointPredicateHistogramPropertyWidget : public AbstractPropertyWidget {
        Q_OBJECT;

    public:
        /**
         * Creates a new PointPredicateHistogramPropertyWidget for the property \a property.
         * \param   property    The property the widget shall handle
         * \param   parent      Parent Qt widget
         */
        PointPredicateHistogramPropertyWidget(PointPredicateHistogramProperty* property, DataContainer* dc, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~PointPredicateHistogramPropertyWidget();

    protected:
        /// Slot called when a predicate histogram's header has changed
        void onHistogramHeaderChanged();

        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty();

    protected slots:
        /// Slot called when the selected predicate has changed.
        void onSelectedPredicateChanged(AbstractPointPredicate* predicate);
        /// Slot called when the hue slider has changed.
        void onHueValueChanged(int value);
        /// Slot called when the saturation slider has changed.
        void onSaturationValueChanged(int value);
        /// Slot called when the add button was clicked.
        void onBtnAddClicked(bool);
        /// Slot called when the reset button was clicked.
        void onBtnResetClicked(bool);

    private:
        /**
         * Creates a new point predicate from the given list item.
         * \param   item    List item to use for determining type of predicate (uses text).
         * \return  
         */
        AbstractPointPredicate* createPredicate(QListWidgetItem* item);

        /**
         * Returns the property correctly casted to PointPredicateHistogramProperty.
         * \return  static_cast<PointPredicateHistogramProperty*>(_property)
         */
        PointPredicateHistogramProperty* getProp() {
            return static_cast<PointPredicateHistogramProperty*>(_property);
        }

        PropertyCollectionWidget* _pcw;             ///< PropertyCollectionWidget to show additional configuration widgets of the selected predicate

        QListWidget* _lwPredicates;                 ///< List widget to select new predicate to create
        QPushButton* _btnAdd;                       ///< Button to add new predicates
        QPushButton* _btnReset;                     ///< Button to reset predicate configuration

        PointPredicateRenderArea* _canvas;          ///< Render area showing the rendered predicate
        AbstractPointPredicate* _selectedPredicate; ///< Pointer to currently selected predicate, may be 0.

        QSlider* _slHue;                            ///< Slider to modify the hue of the currently selected predicate
        QSlider* _slSaturation;                     ///< Slider to modify the saturation of the currently selected predicate
    };

    // explicitly instantiate template, so that it gets registered also over DLL boundaries.
    template class PropertyWidgetRegistrar<PointPredicateHistogramPropertyWidget, PointPredicateHistogramProperty, 10>;

}


#endif // POINTPREDICATEHISTOGRAMPROPERTYWIDGET_H__
