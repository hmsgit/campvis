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

#ifndef FLOATPROPERTYWIDGET_H__
#define FLOATPROPERTYWIDGET_H__

#include "application/gui/adjusterwidgets/doubleadjusterwidget.h"
#include "application/gui/properties/abstractpropertywidget.h"
#include "core/properties/floatingpointproperty.h"

namespace campvis {
    /**
     * Widget for a FloatProperty
     */
    class CAMPVIS_APPLICATION_API FloatPropertyWidget : public AbstractPropertyWidget {
        Q_OBJECT

    public:
        /**
         * Creates a new FloatPropertyWidget for the property \a property.
         * \param   property    The property the widget shall handle
         * \param   dataContainer   DataContainer to use (optional), defaults to nullptr.
         * \param   parent      Parent Qt widget
         */
        FloatPropertyWidget(FloatProperty* property, DataContainer* dataContainer, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~FloatPropertyWidget();

    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty();

    private slots:
        /// Slot getting called when the adjuster's value changes
        void onAdjusterValueChanged(double value);

    private:
        /// Slot getting called when the property's min or max value has changed, so that the widget can be updated.
        virtual void onPropertyMinMaxChanged(const AbstractProperty* property);

        /// Slot getting called when the property's step value has changed, so that the widget can be updated.
        virtual void onPropertyStepChanged(const AbstractProperty* property);

        /**
         * Slot getting called when the number of significant decimal places of the property has
         * changed, so that the widget can be updated.
         */
        virtual void onPropertyDecimalsChanged(const AbstractProperty* property);

        DoubleAdjusterWidget* _adjuster;        ///< Widget allowing the user to change the property's value

    };

// ================================================================================================

    namespace {
        template<size_t SIZE>
        struct VecPropertyWidgetTraits {};

        template<>
        struct VecPropertyWidgetTraits<2> {
            typedef Vec2Property PropertyType;
            typedef cgt::vec2 BaseType;
        };

        template<>
        struct VecPropertyWidgetTraits<3> {
            typedef Vec3Property PropertyType;
            typedef cgt::vec3 BaseType;
        };

        template<>
        struct VecPropertyWidgetTraits<4> {
            typedef Vec4Property PropertyType;
            typedef cgt::vec4 BaseType;
        };
    }

// ================================================================================================

    /**
     * Generic base class for Vec property widgets.
     * Unfortunately Q_OBJECT and templates do not fit together, so we an additional level of 
     * indirection helps as usual...
     */
    template<size_t SIZE>
    class VecPropertyWidget : public AbstractPropertyWidget {
    public:
        enum { size = SIZE };
        typedef typename VecPropertyWidgetTraits<SIZE>::PropertyType PropertyType;

        /**
         * Creates a new VecPropertyWidget for the property \a property.
         * \param   property    The property the widget shall handle
         * \param   parent      Parent Qt widget
         */
        VecPropertyWidget(PropertyType* property, DataContainer* dataContainer = nullptr, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~VecPropertyWidget();

    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty();

        void onValueChangedImpl();

        /// Slot getting called when the property's min or max value has changed, so that the widget can be updated.
        virtual void onPropertyMinMaxChanged(const AbstractProperty* property);

        /// Slot getting called when the property's step value has changed, so that the widget can be updated.
        virtual void onPropertyStepChanged(const AbstractProperty* property);

        /**
         * Slot getting called when the number of significant decimal places of the property has
         * changed, so that the widget can be updated.
         */
        virtual void onPropertyDecimalsChanged(const AbstractProperty* property);

        DoubleAdjusterWidget* _adjusters[size];
    };

// ================================================================================================

    template<size_t SIZE>
    campvis::VecPropertyWidget<SIZE>::VecPropertyWidget(PropertyType* property, DataContainer* dataContainer, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, true, dataContainer, parent)
    {
        for (size_t i = 0; i < size; ++i) {
            _adjusters[i] = new DoubleAdjusterWidget();
            _adjusters[i]->setMinimum(property->getMinValue()[i]);
            _adjusters[i]->setMaximum(property->getMaxValue()[i]);
            _adjusters[i]->setDecimals(property->getDecimals()[i]);
            _adjusters[i]->setSingleStep(property->getStepValue()[i]);
            _adjusters[i]->setValue(property->getValue()[i]);
            addWidget(_adjusters[i]);
        }

        property->s_minMaxChanged.connect(this, &VecPropertyWidget::onPropertyMinMaxChanged);
        property->s_stepChanged.connect(this, &VecPropertyWidget::onPropertyStepChanged);
        property->s_decimalsChanged.connect(this, &VecPropertyWidget::onPropertyDecimalsChanged);
    }

    template<size_t SIZE>
    campvis::VecPropertyWidget<SIZE>::~VecPropertyWidget() {
        PropertyType* property = static_cast<PropertyType*>(_property);

        property->s_minMaxChanged.disconnect(this);
        property->s_stepChanged.disconnect(this);
        property->s_decimalsChanged.disconnect(this);
    }

    template<size_t SIZE>
    void campvis::VecPropertyWidget<SIZE>::updateWidgetFromProperty() {
        PropertyType* prop = static_cast<PropertyType*>(_property);
        for (size_t i = 0; i < size; ++i) {
            _adjusters[i]->blockSignals(true);
            _adjusters[i]->setValue(prop->getValue()[i]);
            _adjusters[i]->blockSignals(false);
        }
    }

    template<size_t SIZE>
    void campvis::VecPropertyWidget<SIZE>::onValueChangedImpl() {
        ++_ignorePropertyUpdates;
        PropertyType* prop = static_cast<PropertyType*>(_property);
        typename VecPropertyWidgetTraits<SIZE>::BaseType newValue;
        for (size_t i = 0; i < size; ++i)
            newValue[i] = _adjusters[i]->value();
        prop->setValue(newValue);
        --_ignorePropertyUpdates;
    }

    template<size_t SIZE>
    void campvis::VecPropertyWidget<SIZE>::onPropertyMinMaxChanged(const AbstractProperty* property) {
        if (_ignorePropertyUpdates == 0) {
            PropertyType* prop = static_cast<PropertyType*>(_property);
            for (size_t i = 0; i < size; ++i) {
                _adjusters[i]->setMinimum(prop->getMinValue()[i]);
                _adjusters[i]->setMaximum(prop->getMaxValue()[i]);
            }
        }
    }

    template<size_t SIZE>
    void campvis::VecPropertyWidget<SIZE>::onPropertyStepChanged(const AbstractProperty* property) {
        if (_ignorePropertyUpdates == 0) {
            PropertyType* prop = static_cast<PropertyType*>(_property);
            for (size_t i = 0; i < size; ++i) {
                _adjusters[i]->setSingleStep(prop->getStepValue()[i]);
            }
        }
    }

    template<size_t SIZE>
    void campvis::VecPropertyWidget<SIZE>::onPropertyDecimalsChanged(const AbstractProperty* property) {
        if (_ignorePropertyUpdates == 0) {
            PropertyType* prop = static_cast<PropertyType*>(_property);
            for (size_t i = 0; i < size; ++i) {
                _adjusters[i]->setDecimals(prop->getDecimals()[i]);
            }
        }
    }

// ================================================================================================

    class CAMPVIS_APPLICATION_API Vec2PropertyWidget : public VecPropertyWidget<2> {
        Q_OBJECT
    public:
        Vec2PropertyWidget(PropertyType* property, DataContainer* dataContainer = nullptr, QWidget* parent = 0);

    private slots:
        void onValueChanged(double value) { onValueChangedImpl(); };
    }; 

// ================================================================================================
    
    class CAMPVIS_APPLICATION_API Vec3PropertyWidget : public VecPropertyWidget<3> {
        Q_OBJECT
    public:
        Vec3PropertyWidget(PropertyType* property, DataContainer* dataContainer = nullptr, QWidget* parent = 0);

        private slots:
            void onValueChanged(double value) { onValueChangedImpl(); };
    }; 

// ================================================================================================

    class CAMPVIS_APPLICATION_API Vec4PropertyWidget : public VecPropertyWidget<4> {
        Q_OBJECT
    public:
        Vec4PropertyWidget(PropertyType* property, DataContainer* dataContainer = nullptr, QWidget* parent = 0);

        private slots:
            void onValueChanged(double value) { onValueChangedImpl(); };
    }; 

}

#endif // FLOATPROPERTYWIDGET_H__
