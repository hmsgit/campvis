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

#ifndef INTADJUSTERWIDGET_H__
#define INTADJUSTERWIDGET_H__

#include "abstractadjusterwidget.h"

namespace campvis {
    /**
     * Widget providing two ways of adjusting integer values.
     *
     * IntAdjusterWidget consists of a slider, which can be used to quickly change integer values, and a spin box,
     * which is better suited for precise adjustments.
     */
    class IntAdjusterWidget : public AbstractAdjusterWidget<int> {
        Q_OBJECT

    public:
        /**
         * Creates a new IntAdjusterWidget.
         * \param   parent      Parent Qt widget
         */
        explicit IntAdjusterWidget(QWidget* parent = 0);

        /**
         * Change the adjuster's current value.
         * \param   value      New value for the adjuster
         */
        void setValue(int value);

        /**
         * Change the adjuster's minimum value.
         * \param   minimum      new minimum value for the adjuster
         */
        void setMinimum(int minimum);

        /**
         * Change the adjuster's maximum value.
         * \param   maximum      new maximum value for the adjuster
         */
        void setMaximum(int maximum);

        /**
         * Change the adjuster's single step value.
         * \param   value      new single step value for the adjuster
         */
        void setSingleStep(int value);

    signals:
        /**
         * This signal is emitted whenever the adjuster's value is changed.
         * \param   value      New value of the adjuster
         */
        void valueChanged(int value);

        /// Signal emitted when the property's minimum value has changed
        void s_minChanged(int minimum);
        /// Signal emitted when the property's maximum value has changed
        void s_maxChanged(int maximum);
        /// Signal emitted when the property's single step value has changed
        void s_singleStepChanged(int value);


    private slots:
        /// Slot getting called when the spin box's value changes
        void onSpinBoxValueChanged(int value);

        /// Slot getting called when the slider's value changes
        void onSliderValueChanged(int value);

        /// Slot getting called when the property's minimum value has changed
        void onMinChanged(int minimum);
        /// Slot getting called when the property's maximum value has changed
        void onMaxChanged(int maximum);
        /// Slot getting called when the property's single step value has changed
        void onSingleStepChanged(int value);

    };

}

#endif // INTADJUSTERWIDGET_H__
