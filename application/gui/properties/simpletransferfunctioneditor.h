// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#ifndef SIMPLETRANSFERFUNCTIONEDITOR_H__
#define SIMPLETRANSFERFUNCTIONEDITOR_H__

#include "application/gui/properties/abstracttransferfunctioneditor.h"
#include <QWidget>

class QGridLayout;
class QLabel;

namespace campvis {
    class ColorPickerWidget;
    class SimpleTransferFunction;

    /**
     * Editor widget for a SimpleTransferFunction.
     */
    class SimpleTransferFunctionEditor : public AbstractTransferFunctionEditor {
        Q_OBJECT;

    public:
        /**
         * Creates a new editor widget for the for the TransferFunctionProperty \a property.
         * \param   tf          The transfer function the editor shall handle.
         * \param   parent      Parent Qt widget
         */
        SimpleTransferFunctionEditor(SimpleTransferFunction* tf, QWidget* parent = 0);

        /**
         * Destructor
         */
        virtual ~SimpleTransferFunctionEditor();

    protected:
        /**
         * Gets called when the property has changed, so that widget can update its state.
         */
        virtual void updateWidgetFromProperty();

    private slots:
        void onLeftColorChanged(const QColor& color);
        void onRightColorChanged(const QColor& color);

    private:
        QGridLayout* _layout;

        QLabel* _lblColorLeft;
        QLabel* _lblColorRight;
        ColorPickerWidget* _cpwColorLeft;
        ColorPickerWidget* _cpwColorRight;
    };
}

#endif // SIMPLETRANSFERFUNCTIONEDITOR_H__
