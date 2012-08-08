#include "boolpropertywidget.h"

#include <QCheckBox>

namespace TUMVis {
    BoolPropertyWidget::BoolPropertyWidget(BoolProperty* property, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, parent)
        , _spinBox(0)
    {
        _spinBox = new QCheckBox(this);
        _spinBox->setChecked(property->getValue());
        
        addWidget(_spinBox);

        connect(_spinBox, SIGNAL(stateChanged(int)), this, SLOT(onStateChanged(int)));
    }

    BoolPropertyWidget::~BoolPropertyWidget() {
    }

    void BoolPropertyWidget::updateWidgetFromProperty() {
        BoolProperty* prop = static_cast<BoolProperty*>(_property);
        _spinBox->blockSignals(true);
        _spinBox->setChecked(prop->getValue());
        _spinBox->blockSignals(false);
    }

    void BoolPropertyWidget::onStateChanged(int  value) {
        _ignorePropertyUpdates = true;
        BoolProperty* prop = static_cast<BoolProperty*>(_property);
        prop->setValue(_spinBox->isChecked());
        _ignorePropertyUpdates = false;
    }

}