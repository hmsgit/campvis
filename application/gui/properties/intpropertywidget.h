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

#ifndef INTPROPERTYWIDGET_H__
#define INTPROPERTYWIDGET_H__

#include "application/gui/adjusterwidgets/intadjusterwidget.h"
#include "application/gui/properties/abstractpropertywidget.h"
#include "core/properties/numericproperty.h"

class QCheckBox;
class QPushButton;
class QTimer;

namespace campvis {
    /**
     * Widget for a IntProperty
     */
    class IntPropertyWidget : public AbstractPropertyWidget {
        Q_OBJECT;

    public:
        /**
         * Creates a new IntPropertyWidget for the property \a property.
         * \param   property    The property the widget shall handle
         * \param   parent      Parent Qt widget
         */
        IntPropertyWidget(IntProperty* property, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~IntPropertyWidget();

    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty();

    private slots:
        void onValueChanged(int value);
        void onIntervalValueChanged(int value);
        void onEnableTimerChanged(int state);
        void onTimer();
        void onBtnSHTClicked();

    private:
        /// Slot getting called when the property's min or max value has changed, so that the widget can be updated.
        virtual void onPropertyMinMaxChanged(const AbstractProperty* property);

        /// Slot getting called when the property's step value has changed, so that the widget can be updated.
        virtual void onPropertyStepChanged(const AbstractProperty* property);

        IntAdjusterWidget* _adjuster;

        QPushButton* _btnShowHideTimer;
        QTimer* _timer;
        QCheckBox* _cbEnableTimer;
        QSpinBox* _sbInterval;

    };

// ================================================================================================

    namespace {
        template<size_t SIZE>
        struct IVecPropertyWidgetTraits {};

        template<>
        struct IVecPropertyWidgetTraits<2> {
            typedef IVec2Property PropertyType;
            typedef tgt::ivec2 BaseType;
        };

        template<>
        struct IVecPropertyWidgetTraits<3> {
            typedef IVec3Property PropertyType;
            typedef tgt::ivec3 BaseType;
        };

        template<>
        struct IVecPropertyWidgetTraits<4> {
            typedef IVec4Property PropertyType;
            typedef tgt::ivec4 BaseType;
        };
    }

// ================================================================================================

    /**
     * Generic base class for IVec property widgets.
     * Unfortunately Q_OBJECT and templates do not fit together, so we an additional level of 
     * indirection helps as usual...
     */
    template<size_t SIZE>
    class IVecPropertyWidget : public AbstractPropertyWidget {
    public:
        enum { size = SIZE };
        typedef typename IVecPropertyWidgetTraits<SIZE>::PropertyType PropertyType;

        /**
         * Creates a new IVecPropertyWidget for the property \a property.
         * \param   property    The property the widget shall handle
         * \param   parent      Parent Qt widget
         */
        IVecPropertyWidget(PropertyType* property, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~IVecPropertyWidget();

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

        IntAdjusterWidget* _adjusters[size];
    };

// ================================================================================================

    template<size_t SIZE>
    campvis::IVecPropertyWidget<SIZE>::IVecPropertyWidget(PropertyType* property, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, true, parent)
    {
        for (size_t i = 0; i < size; ++i) {
            _adjusters[i] = new IntAdjusterWidget;
            _adjusters[i]->setMinimum(property->getMinValue()[i]);
            _adjusters[i]->setMaximum(property->getMaxValue()[i]);
            _adjusters[i]->setSingleStep(property->getStepValue()[i]);
            _adjusters[i]->setValue(property->getValue()[i]);
            addWidget(_adjusters[i]);
        }

        property->s_minMaxChanged.connect(this, &IVecPropertyWidget::onPropertyMinMaxChanged);
        property->s_stepChanged.connect(this, &IVecPropertyWidget::onPropertyStepChanged);
    }

    template<size_t SIZE>
    campvis::IVecPropertyWidget<SIZE>::~IVecPropertyWidget() {
        static_cast<PropertyType*>(_property)->s_minMaxChanged.disconnect(this);
        static_cast<PropertyType*>(_property)->s_stepChanged.disconnect(this);
    }

    template<size_t SIZE>
    void campvis::IVecPropertyWidget<SIZE>::updateWidgetFromProperty() {
        PropertyType* prop = static_cast<PropertyType*>(_property);
        for (size_t i = 0; i < size; ++i) {
            _adjusters[i]->blockSignals(true);
            _adjusters[i]->setValue(prop->getValue()[i]);
            _adjusters[i]->blockSignals(false);
        }
    }

    template<size_t SIZE>
    void campvis::IVecPropertyWidget<SIZE>::onValueChangedImpl() {
        ++_ignorePropertyUpdates;
        PropertyType* prop = static_cast<PropertyType*>(_property);
        typename IVecPropertyWidgetTraits<SIZE>::BaseType newValue;
        for (size_t i = 0; i < size; ++i)
            newValue[i] = _adjusters[i]->value();
        prop->setValue(newValue);
        --_ignorePropertyUpdates;
    }

    template<size_t SIZE>
    void campvis::IVecPropertyWidget<SIZE>::onPropertyMinMaxChanged(const AbstractProperty* property) {
        if (_ignorePropertyUpdates == 0) {
            PropertyType* prop = static_cast<PropertyType*>(_property);
            for (size_t i = 0; i < size; ++i) {
                _adjusters[i]->setMinimum(prop->getMinValue()[i]);
                _adjusters[i]->setMaximum(prop->getMaxValue()[i]);
            }
        }
    }

    template<size_t SIZE>
    void campvis::IVecPropertyWidget<SIZE>::onPropertyStepChanged(const AbstractProperty* property) {
        if (_ignorePropertyUpdates == 0) {
            PropertyType* prop = static_cast<PropertyType*>(_property);
            for (size_t i = 0; i < size; ++i) {
                _adjusters[i]->setSingleStep(prop->getStepValue()[i]);
            }
        }
    }

// ================================================================================================

    class IVec2PropertyWidget : public IVecPropertyWidget<2> {
        Q_OBJECT
    public:
        IVec2PropertyWidget(PropertyType* property, QWidget* parent = 0)
            : IVecPropertyWidget<2>(property, parent)
        {
            for (size_t i = 0; i < size; ++i) {
                connect(_adjusters[i], SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
            }
        }

    private slots:
        void onValueChanged(int value) { onValueChangedImpl(); };
    }; 

// ================================================================================================
    
    class IVec3PropertyWidget : public IVecPropertyWidget<3> {
        Q_OBJECT
    public:
        IVec3PropertyWidget(PropertyType* property, QWidget* parent = 0)
            : IVecPropertyWidget<3>(property, parent)
        {
            for (size_t i = 0; i < size; ++i) {
                connect(_adjusters[i], SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
            }
        }

        private slots:
            void onValueChanged(int value) { onValueChangedImpl(); };
    }; 

// ================================================================================================

    class IVec4PropertyWidget : public IVecPropertyWidget<4> {
        Q_OBJECT
    public:
        IVec4PropertyWidget(PropertyType* property, QWidget* parent = 0)
            : IVecPropertyWidget<4>(property, parent)
        {
            for (size_t i = 0; i < size; ++i) {
                connect(_adjusters[i], SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
            }
        }

        private slots:
            void onValueChanged(int value) { onValueChangedImpl(); };
    }; 

}
#endif // INTPROPERTYWIDGET_H__
