#include "abstracttransferfunctioneditor.h"
#include "core/classification/abstracttransferfunction.h"

namespace TUMVis {

    AbstractTransferFunctionEditor::AbstractTransferFunctionEditor(AbstractTransferFunction* tf, QWidget* parent /*= 0*/)
        : QWidget(parent)
        , _transferFunction(tf)
        , _ignorePropertyUpdates(false)
    {
        _transferFunction->s_changed.connect(this, &AbstractTransferFunctionEditor::onTFChanged);
    }

    AbstractTransferFunctionEditor::~AbstractTransferFunctionEditor() {
        _transferFunction->s_changed.disconnect(this);
    }
    
    void AbstractTransferFunctionEditor::onTFChanged() {
        if (!_ignorePropertyUpdates)
            updateWidgetFromProperty();
    }
}