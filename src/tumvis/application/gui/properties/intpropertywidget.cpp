// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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
        static_cast<IntProperty*>(_property)->s_minMaxChanged.disconnect(this);
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