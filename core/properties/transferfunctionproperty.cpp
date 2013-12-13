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

#include "transferfunctionproperty.h"

namespace campvis {

    const std::string TransferFunctionProperty::loggerCat_ = "CAMPVis.core.datastructures.TransferFunctionProperty";

    TransferFunctionProperty::TransferFunctionProperty(const std::string& name, const std::string& title, AbstractTransferFunction* tf, int invalidationLevel /*= AbstractProcessor::INVALID_RESULT*/)
        : AbstractProperty(name, title, invalidationLevel)
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

        if (_transferFunction != 0) {
            _transferFunction->s_changed.disconnect(this);
            _transferFunction->deinit();
        }
        delete _transferFunction;


        _transferFunction = tf;
        if (_transferFunction != 0)
            _transferFunction->s_changed.connect(this, &TransferFunctionProperty::onTFChanged);

        s_AfterTFReplace(_transferFunction);
    }

    void TransferFunctionProperty::addSharedProperty(AbstractProperty* prop) {
        tgtAssert(false, "Sharing of TF properties not supported!");
    }

}
