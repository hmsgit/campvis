#include "intpropertywidget.h"

namespace TUMVis {
    IntPropertyWidget::IntPropertyWidget(IntProperty* property, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, parent)
        , _spinBox(0)
    {
        _spinBox = new QSpinBox(this);
        _spinBox->setMinimum(property->getMinValue());
        _spinBox->setMaximum(property->getMaxValue());
        _spinBox->setValue(property->getValue());
        
        addWidget(_spinBox);

        connect(_spinBox, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
        property->s_minMaxChanged.connect(this, &IntPropertyWidget::onPropertyMinMaxChanged);
    }

    IntPropertyWidget::~IntPropertyWidget() {

    }

    void IntPropertyWidget::updateWidgetFromProperty() {
        IntProperty* prop = static_cast<IntProperty*>(_property);
        _spinBox->blockSignals(true);
        _spinBox->setValue(prop->getValue());
        _spinBox->blockSignals(false);
    }

    void IntPropertyWidget::onValueChanged(int value) {
        _ignorePropertyUpdates = true;
        IntProperty* prop = static_cast<IntProperty*>(_property);
        prop->setValue(value);
        _ignorePropertyUpdates = false;
    }

    void IntPropertyWidget::onPropertyMinMaxChanged(const AbstractProperty* property) {
        if (!_ignorePropertyUpdates) {
            IntProperty* prop = static_cast<IntProperty*>(_property);
            _spinBox->setMinimum(prop->getMinValue());
            _spinBox->setMaximum(prop->getMaxValue());
        }
    }
}