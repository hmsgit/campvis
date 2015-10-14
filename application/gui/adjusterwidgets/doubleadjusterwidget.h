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

#ifndef DOUBLEADJUSTERWIDGET_H__
#define DOUBLEADJUSTERWIDGET_H__

#include "abstractadjusterwidget.h"
#include "application/applicationapi.h"

namespace campvis {
    /**
     * Widget providing two ways of adjusting double-precision floating-point values.
     *
     * DoubleAdjusterWidget consists of a slider, which can be used to quickly change numeric values, and a spin box,
     * which is better suited for precise adjustments.
     */
    class CAMPVIS_APPLICATION_API DoubleAdjusterWidget : public AbstractAdjusterWidget<double> {

        Q_OBJECT

    public:
        /**
         * Creates a new DoubleAdjusterWidget.
         * \param   parent      parent Qt widget
         */
        explicit DoubleAdjusterWidget(QWidget* parent = 0);

        /**
         * Returns how many decimals the adjuster will use for displaying and doubleerpreting doubles.
         */
        int decimals() const;

        /**
         * Set how many decimals the adjuster will use for displaying and doubleerpreting doubles.
         * \param   prec      number of decimals the adjuster will use
         */
        void setDecimals(int prec);

        /**
         * Change the adjuster's minimum value.
         * \param   minimum      new minimum value for the adjuster
         */
        void setMinimum(double minimum);

        /**
         * Change the adjuster's maximum value.
         * \param   maximum      new maximum value for the adjuster
         */
        void setMaximum(double maximum);

        /**
         * Change the adjuster's single step value.
         * \param   value      new single step value for the adjuster
         */
        void setSingleStep(double value);

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

        /// Signal emitted when the property's minimum value has changed
        void s_minChanged(double minimum);
        /// Signal emitted when the property's maximum value has changed
        void s_maxChanged(double maximum);
        /// Signal emitted when the property's single step value has changed
        void s_singleStepChanged(double value);

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

        /// Slot getting called when the property's minimum value has changed
        void onMinChanged(double minimum);
        /// Slot getting called when the property's maximum value has changed
        void onMaxChanged(double maximum);
        /// Slot getting called when the property's single step value has changed
        void onSingleStepChanged(double value);
    };

}

#endif // DOUBLEADJUSTERWIDGET_H__
