// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#include "intadjusterwidget.h"

#include <QHBoxLayout>

namespace campvis {
    IntAdjusterWidget::IntAdjusterWidget(QWidget* parent /*= 0*/)
        : AbstractAdjusterWidget<int>(parent)
    {
        setSliderProperties(_spinBox->singleStep(), _spinBox->minimum(), _spinBox->maximum());

        connect(this, SIGNAL(s_minChanged(int)), this, SLOT(onMinChanged(int)));
        connect(this, SIGNAL(s_maxChanged(int)), this, SLOT(onMaxChanged(int)));
        connect(this, SIGNAL(s_singleStepChanged(int)), this, SLOT(onSingleStepChanged(int)));

        connect(_spinBox, SIGNAL(valueChanged(int)), this, SLOT(onSpinBoxValueChanged(int)));
        connect(_slider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));
    }

    void IntAdjusterWidget::setValue(int value)
    {
        setValueImpl(value);
    }

    void IntAdjusterWidget::setMinimum(int minimum) {
        emit s_minChanged(minimum);
    }

    void IntAdjusterWidget::setMaximum(int maximum) {
        emit s_maxChanged(maximum);
    }

    void IntAdjusterWidget::setSingleStep(int value) {
        emit s_singleStepChanged(value);
    }

    void IntAdjusterWidget::onSpinBoxValueChanged(int value) {
        onSpinBoxValueChangedImpl(value);
        emit valueChanged(value);
    }

    void IntAdjusterWidget::onSliderValueChanged(int value) {
        onSliderValueChangedImpl(value);
        emit valueChanged(value);
    }

    void IntAdjusterWidget::onMinChanged(int minimum) {
        setMinimumImpl(minimum);
    }

    void IntAdjusterWidget::onMaxChanged(int maximum) {
        setMaximumImpl(maximum);
    }

    void IntAdjusterWidget::onSingleStepChanged(int value) {
        setSingleStepImpl(value);
    }

}
