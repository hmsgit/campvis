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

#include "doubleadjusterwidget.h"

#include <cmath>

namespace campvis {
    DoubleAdjusterWidget::DoubleAdjusterWidget(QWidget* parent /*= 0*/)
        : AbstractAdjusterWidget<double>(parent)
    {
        setSliderProperties(_spinBox->singleStep(), _spinBox->minimum(), _spinBox->maximum());

        connect(_spinBox, SIGNAL(valueChanged(double)), this, SLOT(onSpinBoxValueChanged(double)));
        connect(_slider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));
    }

    void DoubleAdjusterWidget::setValue(double value)
    {
        setValueImpl(value);
    }

    void DoubleAdjusterWidget::setDecimals(int prec)
    {
        _spinBox->setDecimals(prec);
        setSliderProperties(_spinBox->singleStep(), _spinBox->minimum(), _spinBox->maximum());
    }

    void DoubleAdjusterWidget::onSpinBoxValueChanged(double value) {
        onSpinBoxValueChangedImpl(value);
        emit valueChanged(value);
    }

    void DoubleAdjusterWidget::onSliderValueChanged(int value) {
        const double newValue = _spinBox->minimum() + value * _spinBox->singleStep();
        onSliderValueChangedImpl(newValue);
        emit valueChanged(newValue);
    }

    void DoubleAdjusterWidget::setSliderValue(double value) {
        AbstractAdjusterWidget<double>::setSliderValue(std::ceil((value - minimum()) / singleStep()));
    }

    void DoubleAdjusterWidget::setSliderProperties(double stepValue, double minValue, double maxValue) {
        // by default minimum and single step are 0 and 1, respectively, so we don't have to change them
        AbstractAdjusterWidget<double>::setSliderProperties(1, 0, std::ceil((maxValue - minValue) / stepValue));
        setSliderValue(value());
    }
}
