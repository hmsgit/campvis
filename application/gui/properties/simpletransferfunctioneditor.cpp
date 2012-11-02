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
        _ignorePropertyUpdates = true;
        static_cast<SimpleTransferFunction*>(_transferFunction)->setLeftColor(QtColorTools::toTgtColor(_cpwColorLeft->color()));
        _ignorePropertyUpdates = false;
    }

    void SimpleTransferFunctionEditor::onRightColorChanged(const QColor& color) {
        _ignorePropertyUpdates = true;
        static_cast<SimpleTransferFunction*>(_transferFunction)->setRightColor(QtColorTools::toTgtColor(_cpwColorRight->color()));
        _ignorePropertyUpdates = false;
    }


}