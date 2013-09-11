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

#include "abstractadjusterwidget.h"

namespace campvis {
    /**
     * Widget providing two ways of adjusting double-precision floating-point values.
     *
     * DoubleAdjusterWidget consists of a slider, which can be used to quickly change numeric values, and a spin box,
     * which is better suited for precise adjustments.
     */
    class DoubleAdjusterWidget : public AbstractAdjusterWidget<double> {

        Q_OBJECT

    public:
        /**
         * Creates a new DoubleAdjusterWidget.
         * \param   parent      parent Qt widget
         */
        DoubleAdjusterWidget(QWidget* parent = 0);

        /**
         * Set how many decimals the adjuster will use for displaying and interpreting doubles.
         * \param   prec      number of decimals the adjuster will use
         */
        void setDecimals(int prec);

    public slots:
        /**
         * Change the adjuster's current value.
         * \param   value      new value for the adjuster
         */
        void setValue(double value);

    signals:
        /**
         * This signal is emitted whenever the adjuster's value is changed.
         * \param   value      new value of the adjuster
         */
        void valueChanged(double value);

    protected:
        /**
         * Calculate and set the slider's value.
         *
         * The slider's value will be based on the provided current value of the adjuster, as well as its step and
         * minimum value.
         *
         * \param   value       the adjuster's new value
         */
        void setSliderValue(double value);

        /**
         * Calculate and set the slider's properties.
         *
         * The slider's properties will be based on the provided step, minimum, and maximum value of the adjuster.
         *
         * \param   stepValue   the adjuster's step value
         * \param   minValue    the adjuster's minimum value
         * \param   maxValue    the adjuster's maximum value
         */
        void setSliderProperties(double stepValue, double minValue, double maxValue);

    private slots:
        /// Slot getting called when the spin box's value changes
        void onSpinBoxValueChanged(double value);

        /// Slot getting called when the slider's value changes
        void onSliderValueChanged(int value);

    };

}

#endif // DOUBLEADJUSTERWIDGET_H__
