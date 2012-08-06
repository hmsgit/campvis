#include "TransferFunctionProperty.h"

namespace TUMVis {

    const std::string TransferFunctionProperty::loggerCat_ = "TUMVis.core.datastructures.TransferFunctionProperty";

    TransferFunctionProperty::TransferFunctionProperty(const std::string& name, const std::string& title, AbstractTransferFunction* tf, InvalidationLevel il /*= InvalidationLevel::INVALID_RESULT*/)
        : AbstractProperty(name, title, il)
        , _transferFunction(tf)
    {
        tgtAssert(tf != 0, "Assigned transfer function must not be 0.");
        tf->s_changed.connect(this, &TransferFunctionProperty::onTFChanged);
    }

    TransferFunctionProperty::~TransferFunctionProperty() {
        _transferFunction->s_changed.disconnect(this);
        delete _transferFunction;
    }

    AbstractTransferFunction* TransferFunctionProperty::getTF() {
        return _transferFunction;
    }

    void TransferFunctionProperty::onTFChanged() {
        s_changed(this);
    }

    void TransferFunctionProperty::deinit() {
        _transferFunction->deinit();
    }

}
