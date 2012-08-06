#include "transferfunctioneditorfactory.h"

#include "application/gui/properties/abstracttransferfunctioneditor.h"
#include "application/gui/properties/simpletransferfunctioneditor.h"

#include "core/classification/abstracttransferfunction.h"
#include "core/classification/simpletransferfunction.h"

namespace TUMVis {

    AbstractTransferFunctionEditor* TransferFunctionEditorFactory::createEditor(AbstractTransferFunction* tf) {
        tgtAssert(tf != 0, "Transfer function must not be 0.");

        if (SimpleTransferFunction* tester = dynamic_cast<SimpleTransferFunction*>(tf)) {
            return new SimpleTransferFunctionEditor(tester);
        }

        return 0;
    }

}
