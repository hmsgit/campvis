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

#ifndef DOUBLEADJUSTERWIDGET_H__
#define DOUBLEADJUSTERWIDGET_H__

#include <QDoubleSpinBox>
#include <QSlider>

namespace campvis {
    /**
     * Widget providing two ways of adjusting double-precision floating-point values.
     *
     * DoubleAdjusterWidget consists of a slider, which can be used to quickly change numeric values, and a spin box,
     * which is better suited for precise adjustments.
     */
    class DoubleAdjusterWidget : public QWidget {
        Q_OBJECT

    public:
        /**
         * Creates a new DoubleAdjusterWidget.
         * \param   parent      Parent Qt widget
         */
        DoubleAdjusterWidget(QWidget* parent = 0);

        /**
         * Return the adjuster's current value.
         */
        double value() const;

        /**
         * Change the adjuster's minimum value.
         * \param   minimum      New minimum value for the adjuster
         */
        void setMinimum(double minimum);

        /**
         * Change the adjuster's maximum value.
         * \param   maximum      New maximum value for the adjuster
         */
        void setMaximum(double maximum);

        /**
         * Change the adjuster's single step value.
         * \param   value      New single step value for the adjuster
         */
        void setSingleStep(double value);

        /**
         * Set how many decimals the adjuster will use for displaying and interpreting doubles.
         * \param   prec      Number of decimals the adjuster will use
         */
        void setDecimals(int prec);

    public slots:
        /**
         * Change the adjuster's current value.
         * \param   value      New value for the adjuster
         */
        void setValue(double value);

    signals:
        /**
         * This signal is emitted whenever the adjuster's value is changed.
         * \param   value      New value of the adjuster
         */
        void valueChanged(double value);

    private slots:
        /// Slot getting called when the spin box's value changes
        void onSpinBoxValueChanged(double value);
        /// Slot getting called when the slider's value changes
        void onSliderValueChanged(int value);

    private:
        /**
         * Calculate and set the slider's value.
         *
         * The slider's value will be based on the provided current, step, and minimum value of the property.
         *
         * \param   value       The property's current value
         * \param   stepValue   The property's step value
         * \param   minValue    The property's minimum value
         */
        void setSliderValue(double value, double stepValue, double minValue);

        /**
         * Calculate and set the slider's properties.
         *
         * The slider's properties (current and maximum value) will be based on the provided current, step, minimum,
         * and maximum value of the property.
         *
         * \param   value       The property's current value
         * \param   stepValue   The property's step value
         * \param   minValue    The property's minimum value
         * \param   maxValue    The property's maximum value
         */
        void setSliderProperties(double value, double stepValue, double minValue, double maxValue);

        QSlider* _slider;                       ///< Slider allowing the user to quickly change the value
        QDoubleSpinBox* _spinBox;               ///< Spin box displaying the current value

    };

}

#endif // DOUBLEADJUSTERWIDGET_H__
