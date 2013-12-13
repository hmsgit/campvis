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

#include "simpletransferfunctioneditor.h"

#include "application/gui/qtcolortools.h"
#include "core/classification/simpletransferfunction.h"
#include "core/properties/transferfunctionproperty.h"

#include <QGridLayout>
#include <QLabel>

namespace campvis {

    SimpleTransferFunctionEditor::SimpleTransferFunctionEditor(SimpleTransferFunction* tf, QWidget* parent /*= 0*/)
        : AbstractTransferFunctionEditor(tf, parent)
        , _layout(0)
        , _lblColorLeft(0)
        , _lblColorRight(0)
        , _cpwColorLeft(0)
        , _cpwColorRight(0)
    {
        _layout = new QGridLayout(this);
        setLayout(_layout);

        _lblColorLeft = new QLabel("Left Color: ", this);
        _lblColorRight = new QLabel("Right Color: ", this);
        _cpwColorLeft = new ColorPickerWidget(QtColorTools::toQColor(tf->getLeftColor()), this);
        _cpwColorRight = new ColorPickerWidget(QtColorTools::toQColor(tf->getRightColor()), this);

        _layout->addWidget(_lblColorLeft, 0, 0);
        _layout->addWidget(_cpwColorLeft, 0, 1);
        _layout->addWidget(_lblColorRight, 1, 0);
        _layout->addWidget(_cpwColorRight, 1, 1);

        connect(_cpwColorLeft, SIGNAL(colorChanged(const QColor&)), this, SLOT(onLeftColorChanged(const QColor&)));
        connect(_cpwColorRight, SIGNAL(colorChanged(const QColor&)), this, SLOT(onRightColorChanged(const QColor&)));
    }

    SimpleTransferFunctionEditor::~SimpleTransferFunctionEditor() {
    }

    void SimpleTransferFunctionEditor::updateWidgetFromProperty() {
        SimpleTransferFunction* stf = static_cast<SimpleTransferFunction*>(_transferFunction);
        _cpwColorLeft->blockSignals(true);
        _cpwColorRight->blockSignals(true);
        _cpwColorLeft->setColor(QtColorTools::toQColor(static_cast<SimpleTransferFunction*>(_transferFunction)->getLeftColor()));
        _cpwColorRight->setColor(QtColorTools::toQColor(static_cast<SimpleTransferFunction*>(_transferFunction)->getRightColor()));
        _cpwColorLeft->blockSignals(false);
        _cpwColorRight->blockSignals(false);
    }

    void SimpleTransferFunctionEditor::onLeftColorChanged(const QColor& color) {
        ++_ignorePropertyUpdates;
        static_cast<SimpleTransferFunction*>(_transferFunction)->setLeftColor(QtColorTools::toTgtColor(_cpwColorLeft->color()));
        --_ignorePropertyUpdates;
    }

    void SimpleTransferFunctionEditor::onRightColorChanged(const QColor& color) {
        ++_ignorePropertyUpdates;
        static_cast<SimpleTransferFunction*>(_transferFunction)->setRightColor(QtColorTools::toTgtColor(_cpwColorRight->color()));
        --_ignorePropertyUpdates;
    }


}