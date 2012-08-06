#include "transferfunctionpropertywidget.h"

namespace TUMVis {
    TransferFunctionPropertyWidget::TransferFunctionPropertyWidget(TransferFunctionProperty* property, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, parent)
        , _widget(0)
        , _gridLayout(0)
        , _lblDomain(0)
        , _spinDomainLeft(0)
        , _spinDomainRight(0)
        , _btnEditTF(0)
    {
        _widget = new QWidget(this);
        _gridLayout = new QGridLayout(_widget);
        _widget->setLayout(_gridLayout);

        _lblDomain = new QLabel("Intensity Domain: ", _widget);
        _gridLayout->addWidget(_lblDomain, 0, 0);


        _spinDomainLeft = new QDoubleSpinBox(_widget);
        _spinDomainLeft->setMinimum(0);
        _spinDomainLeft->setDecimals(2);
        _spinDomainLeft->setSingleStep(0.01);
        _gridLayout->addWidget(_spinDomainLeft, 0, 1);

        _spinDomainRight = new QDoubleSpinBox(_widget);
        _spinDomainRight->setMaximum(1);
        _spinDomainRight->setDecimals(2);
        _spinDomainRight->setSingleStep(0.01);
        _gridLayout->addWidget(_spinDomainRight, 0, 2);

        updateWidgetFromProperty();

        _btnEditTF = new QPushButton("Edit Transfer Function", _widget);
        _gridLayout->addWidget(_btnEditTF, 1, 0, 1, 3);

        addWidget(_widget);

        connect(_spinDomainLeft, SIGNAL(valueChanged(double)), this, SLOT(onDomainChanged(double)));
        connect(_spinDomainRight, SIGNAL(valueChanged(double)), this, SLOT(onDomainChanged(double)));
        connect(_btnEditTF, SIGNAL(clicked(bool)), this, SLOT(onEditClicked(bool)));
    }

    TransferFunctionPropertyWidget::~TransferFunctionPropertyWidget() {

    }

    void TransferFunctionPropertyWidget::updateWidgetFromProperty() {
        TransferFunctionProperty* prop = static_cast<TransferFunctionProperty*>(_property);
        AbstractTransferFunction* tf = prop->getTF();
        const tgt::vec2& domain = tf->getIntensityDomain();

        _spinDomainLeft->blockSignals(true);
        _spinDomainLeft->setMaximum(domain.y);
        _spinDomainLeft->setValue(domain.x);
        _spinDomainLeft->blockSignals(false);

        _spinDomainRight->blockSignals(true);
        _spinDomainRight->setMinimum(domain.x);
        _spinDomainRight->setValue(domain.y);
        _spinDomainRight->blockSignals(false);
    }

    void TransferFunctionPropertyWidget::onDomainChanged(double value) {
        TransferFunctionProperty* prop = static_cast<TransferFunctionProperty*>(_property);
        _ignorePropertyUpdates = true;
        _spinDomainLeft->setMaximum(_spinDomainRight->value());
        _spinDomainRight->setMinimum(_spinDomainLeft->value());
        tgt::vec2 newDomain(static_cast<float>(_spinDomainLeft->value()), static_cast<float>(_spinDomainRight->value()));
        prop->getTF()->setIntensityDomain(newDomain);
        _ignorePropertyUpdates = false;
    }

    void TransferFunctionPropertyWidget::onEditClicked(bool checked) {
        // open editor window
    }


}