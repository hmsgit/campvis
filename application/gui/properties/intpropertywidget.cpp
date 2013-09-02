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

#include "intpropertywidget.h"

#include <QCheckBox>
#include <QTimer>
#include <QGridLayout>
#include <QWidget>

namespace campvis {
    IntPropertyWidget::IntPropertyWidget(IntProperty* property, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, false, parent)
        , _spinBox(0)
        , _timer(0)
    {
        _timer = new QTimer(this);
        _timer->setSingleShot(false);

        QWidget* widget = new QWidget(this);
        QGridLayout* layout = new QGridLayout(widget);
        widget->setLayout(layout);

        _spinBox = new QSpinBox(widget);
        _spinBox->setMinimum(property->getMinValue());
        _spinBox->setMaximum(property->getMaxValue());
        _spinBox->setValue(property->getValue());
        layout->addWidget(_spinBox, 0, 0, 1, 2);

        _cbEnableTimer = new QCheckBox("Enable Timer", widget);
        layout->addWidget(_cbEnableTimer, 1, 0);

        _sbInterval = new QSpinBox(widget);
        _sbInterval->setMinimum(1);
        _sbInterval->setMaximum(2000);
        _sbInterval->setValue(50);
        layout->addWidget(_sbInterval, 1, 1);

        addWidget(widget);

        connect(_timer, SIGNAL(timeout()), this, SLOT(onTimer()));
        connect(_spinBox, SIGNAL(valueChanged(int)), this, SLOT(onValueChanged(int)));
        connect(_cbEnableTimer, SIGNAL(stateChanged(int)), this, SLOT(onEnableTimerChanged(int)));
        connect(_sbInterval, SIGNAL(valueChanged(int)), this, SLOT(onIntervalValueChanged(int)));
        property->s_minMaxChanged.connect(this, &IntPropertyWidget::onPropertyMinMaxChanged);
    }

    IntPropertyWidget::~IntPropertyWidget() {
        static_cast<IntProperty*>(_property)->s_minMaxChanged.disconnect(this);
    }

    void IntPropertyWidget::updateWidgetFromProperty() {
        IntProperty* prop = static_cast<IntProperty*>(_property);
        _spinBox->blockSignals(true);
        _spinBox->setValue(prop->getValue());
        _spinBox->blockSignals(false);
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
            _spinBox->setMinimum(prop->getMinValue());
            _spinBox->setMaximum(prop->getMaxValue());
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

}