// ================================================================================================
// 
// This file is part of the CAMPVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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

#include "TransferFunctionProperty.h"

namespace campvis {

    const std::string TransferFunctionProperty::loggerCat_ = "CAMPVis.core.datastructures.TransferFunctionProperty";

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

    void TransferFunctionProperty::replaceTF(AbstractTransferFunction* tf) {
        tgtAssert(tf != 0, "Transfer function must not be 0.");
        s_BeforeTFReplace(_transferFunction);
        delete _transferFunction;
        _transferFunction = tf;
        s_AfterTFReplace(_transferFunction);
    }

}
