// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#ifndef FLOATPROPERTYWIDGET_H__
#define FLOATPROPERTYWIDGET_H__

#include "application/gui/properties/abstractpropertywidget.h"
#include "application/gui/doubleadjusterwidget.h"
#include "core/properties/numericproperty.h"

namespace campvis {
    /**
     * Widget for a FloatProperty
     */
    class FloatPropertyWidget : public AbstractPropertyWidget {
        Q_OBJECT

    public:
        /**
         * Creates a new FloatPropertyWidget for the property \a property.
         * \param   property    The property the widget shall handle
         * \param   parent      Parent Qt widget
         */
        FloatPropertyWidget(FloatProperty* property, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~FloatPropertyWidget();

    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty();

    signals:
        /// Signal emitted when the property's value changes
        void propertyValueChanged(double value);

    private slots:
        /// Slot getting called when the adjuster's value changes
        void onAdjusterValueChanged(double value);

    private:
        /// Slot getting called when the property's min or max value has changed, so that the widget can be updated.
        virtual void onPropertyMinMaxChanged(const AbstractProperty* property);

        DoubleAdjusterWidget* _adjuster;        ///< Widget allowing the user to change the property's value

    };

// ================================================================================================

    namespace {
        template<size_t SIZE>
        struct VecPropertyWidgetTraits {};

        template<>
        struct VecPropertyWidgetTraits<2> {
            typedef Vec2Property PropertyType;
            typedef tgt::vec2 BaseType;
        };

        template<>
        struct VecPropertyWidgetTraits<3> {
            typedef Vec3Property PropertyType;
            typedef tgt::vec3 BaseType;
        };

        template<>
        struct VecPropertyWidgetTraits<4> {
            typedef Vec4Property PropertyType;
            typedef tgt::vec4 BaseType;
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
        VecPropertyWidget(PropertyType* property, QWidget* parent = 0);

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

        DoubleAdjusterWidget* _adjusters[size];
    };

// ================================================================================================

    template<size_t SIZE>
    campvis::VecPropertyWidget<SIZE>::VecPropertyWidget(PropertyType* property, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, true, parent)
    {
        for (size_t i = 0; i < size; ++i) {
            _adjusters[i] = new DoubleAdjusterWidget();
            _adjusters[i]->setMinimum(property->getMinValue()[i]);
            _adjusters[i]->setMaximum(property->getMaxValue()[i]);
            _adjusters[i]->setDecimals(3);
            _adjusters[i]->setSingleStep(0.01);
            _adjusters[i]->setValue(property->getValue()[i]);
            addWidget(_adjusters[i]);
        }

        property->s_minMaxChanged.connect(this, &VecPropertyWidget::onPropertyMinMaxChanged);

    }

    template<size_t SIZE>
    campvis::VecPropertyWidget<SIZE>::~VecPropertyWidget() {
        static_cast<PropertyType*>(_property)->s_minMaxChanged.disconnect(this);
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

// ================================================================================================

    class Vec2PropertyWidget : public VecPropertyWidget<2> {
        Q_OBJECT
    public:
        Vec2PropertyWidget(PropertyType* property, QWidget* parent = 0)
            : VecPropertyWidget<2>(property, parent)
        {
            for (size_t i = 0; i < size; ++i) {
                connect(_adjusters[i], SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
            }
        }

    private slots:
        void onValueChanged(double value) { onValueChangedImpl(); };
    }; 

// ================================================================================================
    
    class Vec3PropertyWidget : public VecPropertyWidget<3> {
        Q_OBJECT
    public:
        Vec3PropertyWidget(PropertyType* property, QWidget* parent = 0)
            : VecPropertyWidget<3>(property, parent)
        {
            for (size_t i = 0; i < size; ++i) {
                connect(_adjusters[i], SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
            }
        }

        private slots:
            void onValueChanged(double value) { onValueChangedImpl(); };
    }; 

// ================================================================================================

    class Vec4PropertyWidget : public VecPropertyWidget<4> {
        Q_OBJECT
    public:
        Vec4PropertyWidget(PropertyType* property, QWidget* parent = 0)
            : VecPropertyWidget<4>(property, parent)
        {
            for (size_t i = 0; i < size; ++i) {
                connect(_adjusters[i], SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
            }
        }

        private slots:
            void onValueChanged(double value) { onValueChangedImpl(); };
    }; 

}

#endif // FLOATPROPERTYWIDGET_H__
