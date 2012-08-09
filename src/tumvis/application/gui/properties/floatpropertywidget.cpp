#include "floatpropertywidget.h"

namespace TUMVis {
    FloatPropertyWidget::FloatPropertyWidget(FloatProperty* property, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, parent)
        , _spinBox(0)
    {
        _spinBox = new QDoubleSpinBox(this);
        _spinBox->setMinimum(property->getMinValue());
        _spinBox->setMaximum(property->getMaxValue());
        _spinBox->setDecimals(3);
        _spinBox->setSingleStep(0.01);
        _spinBox->setValue(property->getValue());
        
        addWidget(_spinBox);

        connect(_spinBox, SIGNAL(valueChanged(double)), this, SLOT(onValueChanged(double)));
        property->s_minMaxChanged.connect(this, &FloatPropertyWidget::onPropertyMinMaxChanged);
    }

    FloatPropertyWidget::~FloatPropertyWidget() {
        static_cast<FloatProperty*>(_property)->s_minMaxChanged.disconnect(this);
    }

    void FloatPropertyWidget::updateWidgetFromProperty() {
        FloatProperty* prop = static_cast<FloatProperty*>(_property);
        _spinBox->blockSignals(true);
        _spinBox->setValue(prop->getValue());
        _spinBox->blockSignals(false);
    }

    void FloatPropertyWidget::onValueChanged(double value) {
        _ignorePropertyUpdates = true;
        FloatProperty* prop = static_cast<FloatProperty*>(_property);
        prop->setValue(value);
        _ignorePropertyUpdates = false;
    }

    void FloatPropertyWidget::onPropertyMinMaxChanged(const AbstractProperty* property) {
        if (!_ignorePropertyUpdates) {
            FloatProperty* prop = static_cast<FloatProperty*>(_property);
            _spinBox->setMinimum(prop->getMinValue());
            _spinBox->setMaximum(prop->getMaxValue());
        }
    }
}