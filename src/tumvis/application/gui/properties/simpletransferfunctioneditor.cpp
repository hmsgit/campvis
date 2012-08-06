#include "simpletransferfunctioneditor.h"

#include "application/gui/qtcolortools.h"
#include "core/classification/simpletransferfunction.h"
#include "core/properties/transferfunctionproperty.h"

#include <QGridLayout>
#include <QLabel>

namespace TUMVis {

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