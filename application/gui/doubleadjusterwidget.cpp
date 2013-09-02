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

#include "doubleadjusterwidget.h"

#include <QHBoxLayout>

#include <cmath>

namespace campvis {
    DoubleAdjusterWidget::DoubleAdjusterWidget(QWidget* parent /*= 0*/)
        : QWidget(parent)
        , _slider(0)
        , _spinBox(0)
    {
        _spinBox = new QDoubleSpinBox();
        _spinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);

        _slider = new QSlider(Qt::Horizontal);
        setSliderProperties(_spinBox->value(), _spinBox->singleStep(), _spinBox->minimum(), _spinBox->maximum());

        QHBoxLayout* layout = new QHBoxLayout;
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(_slider);
        layout->addWidget(_spinBox);
        setLayout(layout);

        connect(_spinBox, SIGNAL(valueChanged(double)), this, SLOT(onSpinBoxValueChanged(double)));
        connect(_slider, SIGNAL(valueChanged(int)), this, SLOT(onSliderValueChanged(int)));
    }

    double DoubleAdjusterWidget::value() const
    {
        return _spinBox->value();
    }

    void DoubleAdjusterWidget::setValue(double value)
    {
        _spinBox->setValue(value);
        setSliderValue(value, _spinBox->singleStep(), _spinBox->minimum());
    }

    void DoubleAdjusterWidget::setMinimum(double minimum)
    {
        _spinBox->setMinimum(minimum);
        setSliderProperties(_spinBox->value(), _spinBox->singleStep(), minimum, _spinBox->maximum());
    }

    void DoubleAdjusterWidget::setMaximum(double maximum)
    {
        _spinBox->setMaximum(maximum);
        setSliderProperties(_spinBox->value(), _spinBox->singleStep(), _spinBox->minimum(), maximum);
    }

    void DoubleAdjusterWidget::setSingleStep(double value)
    {
        _spinBox->setSingleStep(value);
        setSliderProperties(_spinBox->value(), value, _spinBox->minimum(), _spinBox->maximum());
    }

    void DoubleAdjusterWidget::setDecimals(int prec)
    {
        _spinBox->setDecimals(prec);
        setSliderProperties(_spinBox->value(), _spinBox->singleStep(), _spinBox->minimum(), _spinBox->maximum());
    }

    void DoubleAdjusterWidget::onSpinBoxValueChanged(double value) {
        emit valueChanged(value);
        setSliderValue(value, _spinBox->singleStep(), _spinBox->minimum());
    }

    void DoubleAdjusterWidget::onSliderValueChanged(int value) {
        const double newValue = _spinBox->minimum() + value * _spinBox->singleStep();

        _spinBox->blockSignals(true);
        _spinBox->setValue(newValue);
        _spinBox->blockSignals(false);

        emit valueChanged(newValue);
    }

    void DoubleAdjusterWidget::setSliderValue(double value, double stepValue, double minValue) {
        _slider->blockSignals(true);
        _slider->setValue(std::ceil((value - minValue) / stepValue));
        _slider->blockSignals(false);
    }

    void DoubleAdjusterWidget::setSliderProperties(double value, double stepValue, double minValue, double maxValue) {
        // by default minimum and single step are 0 and 1, respectively, so we don't have to change them
        _slider->setMaximum(std::ceil((maxValue - minValue) / stepValue));
        setSliderValue(value, stepValue, minValue);
    }
}
