// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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

#ifndef ABSTRACTADJUSTERWIDGET_H__
#define ABSTRACTADJUSTERWIDGET_H__

#include <QDoubleSpinBox>
#include <QHBoxLayout>
#include <QSlider>
#include <QSpinBox>

namespace campvis {

    namespace {
        template<typename T>
        struct AdjusterWidgetTraits {};

        template<>
        struct AdjusterWidgetTraits<double> {
            typedef QDoubleSpinBox SpinBoxType;
        };

        template<>
        struct AdjusterWidgetTraits<int> {
            typedef QSpinBox SpinBoxType;
        };
    }

// ================================================================================================

    /**
     * Base class for widgets providing two ways of adjusting numeric values.
     *
     * AbstractAdjusterWidget consists of a slider, which can be used to quickly change numeric values, and a spin box,
     * which is better suited for precise adjustments.
     */
    template<typename T>
    class AbstractAdjusterWidget : public QWidget {

    public:
        /**
         * Creates a new AbstractAdjusterWidget.
         * \param   parent      parent Qt widget
         */
        AbstractAdjusterWidget(QWidget* parent = 0);

        /**
         * Return the adjuster's current value.
         */
        T value() const;

        /**
         * Return the adjuster's minimum value.
         */
        T minimum() const;

        /**
         * Change the adjuster's minimum value.
         * \param   minimum      new minimum value for the adjuster
         */
        void setMinimum(T minimum);

        /**
         * Return the adjuster's maximum value.
         */
        T maximum() const;

        /**
         * Change the adjuster's maximum value.
         * \param   maximum      new maximum value for the adjuster
         */
        void setMaximum(T maximum);

        /**
         * Return the adjuster's single step value.
         */
        T singleStep() const;

        /**
         * Change the adjuster's single step value.
         * \param   value      new single step value for the adjuster
         */
        void setSingleStep(T value);

    protected:
        /**
         * Change the adjuster's current value.
         * \param   value      new value for the adjuster
         */
        void setValueImpl(T value);

        /// Slot getting called when the spin box's value changes
        void onSpinBoxValueChangedImpl(T value);

        /// Slot getting called when the slider's value changes
        void onSliderValueChangedImpl(T value);

        /**
         * Set the slider's value.
         *
         * The slider's value will be set to \p value.
         *
         * \param   value       the slider's new value
         */
        virtual void setSliderValue(T value);

        /**
         * Set the slider's properties.
         *
         * The slider's step, minimum, and maximum value will be updated with the provided arguments.
         *
         * \param   stepValue   the slider's new step value
         * \param   minValue    the slider's new minimum value
         * \param   maxValue    the slider's new maximum value
         */
        virtual void setSliderProperties(T stepValue, T minValue, T maxValue);

        QSlider* _slider;                                          ///< Slider allowing the user to quickly change the value
        typename AdjusterWidgetTraits<T>::SpinBoxType* _spinBox;   ///< Spin box displaying the current value

    };

// ================================================================================================

    template<typename T>
    AbstractAdjusterWidget<T>::AbstractAdjusterWidget(QWidget* parent /*= 0*/)
        : QWidget(parent)
        , _slider(0)
        , _spinBox(0)
    {
        _spinBox = new typename AdjusterWidgetTraits<T>::SpinBoxType;
        _spinBox->setSizePolicy(QSizePolicy::Fixed, QSizePolicy::Preferred);
        _slider = new QSlider(Qt::Horizontal);

        QHBoxLayout* layout = new QHBoxLayout;
        layout->setContentsMargins(0, 0, 0, 0);
        layout->addWidget(_slider);
        layout->addWidget(_spinBox);
        setLayout(layout);
    }

    template<typename T>
    T AbstractAdjusterWidget<T>::value() const
    {
        return _spinBox->value();
    }

    template<typename T>
    void AbstractAdjusterWidget<T>::setValueImpl(T value)
    {
        _spinBox->setValue(value);
        setSliderValue(value);
    }

    template<typename T>
    T AbstractAdjusterWidget<T>::minimum() const
    {
        return _spinBox->minimum();
    }

    template<typename T>
    void AbstractAdjusterWidget<T>::setMinimum(T minimum)
    {
        _spinBox->setMinimum(minimum);
        setSliderProperties(_spinBox->singleStep(), minimum, _spinBox->maximum());
    }

    template<typename T>
    T AbstractAdjusterWidget<T>::maximum() const
    {
        return _spinBox->maximum();
    }

    template<typename T>
    void AbstractAdjusterWidget<T>::setMaximum(T maximum)
    {
        _spinBox->setMaximum(maximum);
        setSliderProperties(_spinBox->singleStep(), _spinBox->minimum(), maximum);
    }

    template<typename T>
    T AbstractAdjusterWidget<T>::singleStep() const
    {
        return _spinBox->singleStep();
    }

    template<typename T>
    void AbstractAdjusterWidget<T>::setSingleStep(T value)
    {
        _spinBox->setSingleStep(value);
        setSliderProperties(value, _spinBox->minimum(), _spinBox->maximum());
    }

    template<typename T>
    void AbstractAdjusterWidget<T>::onSpinBoxValueChangedImpl(T value) {
        setSliderValue(value);
    }

    template<typename T>
    void AbstractAdjusterWidget<T>::onSliderValueChangedImpl(T value) {
        _spinBox->blockSignals(true);
        _spinBox->setValue(value);
        _spinBox->blockSignals(false);
    }

    template<typename T>
    void AbstractAdjusterWidget<T>::setSliderValue(T value) {
        _slider->blockSignals(true);
        _slider->setValue(value);
        _slider->blockSignals(false);
    }

    template<typename T>
    void AbstractAdjusterWidget<T>::setSliderProperties(T stepValue, T minValue, T maxValue) {
        _slider->blockSignals(true);
        _slider->setMinimum(minValue);
        _slider->setMaximum(maxValue);
        _slider->setSingleStep(stepValue);
        _slider->blockSignals(false);
    }

}

#endif // ABSTRACTADJUSTERWIDGET_H__