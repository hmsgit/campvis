// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

#include "intpropertywidget.h"

#include <QCheckBox>
#include <QTimer>
#include <QGridLayout>
#include <QPushButton>
#include <QWidget>

namespace campvis {
    IntPropertyWidget::IntPropertyWidget(IntProperty* property, DataContainer* dataContainer, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, false, dataContainer, parent)
        , _adjuster(0)
        , _timer(0)
    {
        _timer = new QTimer(this);
        _timer->setSingleShot(false);

        QWidget* widget = new QWidget(this);
        QGridLayout* layout = new QGridLayout(widget);
        layout->setSpacing(2);
        layout->setMargin(0);
        widget->setLayout(layout);

        _adjuster = new IntAdjusterWidget;
        _adjuster->setMinimum(property->getMinValue());
        _adjuster->setMaximum(property->getMaxValue());
        _adjuster->setSingleStep(property->getStepValue());
        _adjuster->setValue(property->getValue());
        layout->addWidget(_adjuster, 0, 0, 1, 2);

        _btnShowHideTimer = new QPushButton(tr("S"));
        _btnShowHideTimer->setFixedWidth(16);
        layout->addWidget(_btnShowHideTimer, 0, 2);

        _cbEnableTimer = new QCheckBox("Enable Timer", widget);
        _cbEnableTimer->setVisible(false);
        layout->addWidget(_cbEnableTimer, 1, 0);

        _sbInterval = new QSpinBox(widget);
        _sbInterval->setMinimum(1);
        _sbInterval->setMaximum(2000);
        _sbInterval->setValue(50);
        _sbInterval->setVisible(false);
        layout->addWidget(_sbInterval, 1, 1, 1, 2);

        addWidget(widget);

        connect(_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
        connect(_adjuster, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
        connect(_cbEnableTimer, SIGNAL(stateChanged(int)), this, SLOT(onEnableTimerChanged(int)));
        connect(_sbInterval, SIGNAL(valueChanged(int)), this, SLOT(onIntervalValueChanged(int)));
        connect(_btnShowHideTimer, SIGNAL(clicked()), this, SLOT(onBtnSHTClicked()));
        property->s_minMaxChanged.connect(this, &IntPropertyWidget::onPropertyMinMaxChanged);
        property->s_stepChanged.connect(this, &IntPropertyWidget::onPropertyStepChanged);
    }

    IntPropertyWidget::~IntPropertyWidget() {
        static_cast<IntProperty*>(_property)->s_minMaxChanged.disconnect(this);
        static_cast<IntProperty*>(_property)->s_stepChanged.disconnect(this);
    }

    void IntPropertyWidget::updateWidgetFromProperty() {
        IntProperty* prop = static_cast<IntProperty*>(_property);
        _adjuster->blockSignals(true);
        if (_adjuster->value() != prop->getValue())
            _adjuster->setValue(prop->getValue());
        _adjuster->blockSignals(false);
    }

    void IntPropertyWidget::onValueChanged(int value) {
        ++_ignorePropertyUpdates;
        IntProperty* prop = static_cast<IntProperty*>(_property);
        prop->setValue(value);
        --_ignorePropertyUpdates;
    }

    void IntPropertyWidget::onPropertyMinMaxChanged(const AbstractProperty* property) {
        if (_ignorePropertyUpdates == 0) {
            IntProperty* prop = static_cast<IntProperty*>(_property);
            if (_adjuster->minimum() != prop->getMinValue())
                _adjuster->setMinimum(prop->getMinValue());
            if (_adjuster->maximum() != prop->getMaxValue())
                _adjuster->setMaximum(prop->getMaxValue());
        }
    }

    void IntPropertyWidget::onPropertyStepChanged(const AbstractProperty* property) {
        if (_ignorePropertyUpdates == 0) {
            IntProperty* prop = static_cast<IntProperty*>(_property);
            _adjuster->setSingleStep(prop->getStepValue());
        }
    }

    void IntPropertyWidget::onIntervalValueChanged(int value) {
        _timer->setInterval(value);
    }

    void IntPropertyWidget::onEnableTimerChanged(int state) {
        if (state == Qt::Checked) {
            _timer->setInterval(_sbInterval->value());
            _timer->start();
        }
        else if (state == Qt::Unchecked) {
            _timer->stop();
        }
    }

    void IntPropertyWidget::onTimer() {
        IntProperty* prop = static_cast<IntProperty*>(_property);
        if (prop->getValue() < prop->getMaxValue())
            prop->increment();
        else
            prop->setValue(prop->getMinValue());
    }

    void IntPropertyWidget::onBtnSHTClicked() {
        _cbEnableTimer->setVisible(! _cbEnableTimer->isVisible());
        _sbInterval->setVisible(! _sbInterval->isVisible());
        _btnShowHideTimer->setText(_sbInterval->isVisible() ? tr("H") : tr("S"));
    }

}