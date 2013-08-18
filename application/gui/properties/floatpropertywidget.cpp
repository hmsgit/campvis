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

#include "floatpropertywidget.h"

#include <cmath>

namespace campvis {
    FloatPropertyWidget::FloatPropertyWidget(FloatProperty* property, QWidget* parent /*= 0*/)
        : AbstractPropertyWidget(property, parent)
        , _slider(0)
        , _spinBox(0)
    {
        const float& maxValue = property->getMaxValue();
        const float& minValue = property->getMinValue();
        const float& value = property->getValue();
        const float stepValue = 0.01f;

        _slider = new QSlider(Qt::Horizontal);
        setSliderProperties(value, stepValue, minValue, maxValue);

        _spinBox = new QDoubleSpinBox(this);
        _spinBox->setMinimum(minValue);
        _spinBox->setMaximum(maxValue);
        _spinBox->setDecimals(3);
        _spinBox->setSingleStep(stepValue);
        _spinBox->setValue(value);
        _spinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

        addWidget(_slider);
        addWidget(_spinBox);

        connect(_spinBox, SIGNAL(valueChanged(double)), this, SLOT(onSpinBoxValueChanged(double)));
        connect(_slider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));
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
        setSliderValue(prop->getValue(), 0.01f, prop->getMinValue());
    }

    void FloatPropertyWidget::onSpinBoxValueChanged(double value) {
        ++_ignorePropertyUpdates;
        FloatProperty* prop = static_cast<FloatProperty*>(_property);
        prop->setValue(value);
        setSliderValue(value, 0.01f, prop->getMinValue());
        --_ignorePropertyUpdates;
    }

    void FloatPropertyWidget::onSliderValueChanged(int value) {
        FloatProperty* prop = static_cast<FloatProperty*>(_property);

        ++_ignorePropertyUpdates;
        const float newPropValue = prop->getMinValue() + value * 0.01;
        prop->setValue(newPropValue);
        _spinBox->blockSignals(true);
        _spinBox->setValue(newPropValue);
        _spinBox->blockSignals(false);
        --_ignorePropertyUpdates;
    }

    void FloatPropertyWidget::onPropertyMinMaxChanged(const AbstractProperty* property) {
        if (_ignorePropertyUpdates == 0) {
            FloatProperty* prop = static_cast<FloatProperty*>(_property);
            const float& maxValue = prop->getMaxValue();
            const float& minValue = prop->getMinValue();

            _spinBox->setMinimum(minValue);
            _spinBox->setMaximum(maxValue);
            setSliderProperties(prop->getValue(), 0.01f, minValue, maxValue);
        }
    }

    void FloatPropertyWidget::setSliderValue(float value, float stepValue, float minValue) {
        _slider->blockSignals(true);
        _slider->setValue(std::ceil((value - minValue) / stepValue));
        _slider->blockSignals(false);
    }

    void FloatPropertyWidget::setSliderProperties(float value, float stepValue, float minValue, float maxValue) {
        // by default minimum and single step are 0 and 1, respectively, so we don't have to change them
        _slider->setMaximum(std::ceil((maxValue - minValue) / stepValue));
        setSliderValue(value, stepValue, minValue);
    }
}
