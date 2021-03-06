// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#include "transferfunctioneditorfactory.h"

#include "application/gui/properties/abstracttransferfunctioneditor.h"
#include "application/gui/properties/geometry1dtransferfunctioneditor.h"
#include "application/gui/properties/geometry2dtransferfunctioneditor.h"
#include "application/gui/properties/simpletransferfunctioneditor.h"

#include "core/classification/abstracttransferfunction.h"
#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/geometry2dtransferfunction.h"
#include "core/classification/simpletransferfunction.h"

#include "core/properties/transferfunctionproperty.h"

namespace campvis {

    AbstractTransferFunctionEditor* TransferFunctionEditorFactory::createEditor(TransferFunctionProperty* prop) {
        cgtAssert(prop != 0, "Property must not be 0.");

        AbstractTransferFunction* tf = prop->getTF();
        cgtAssert(tf != 0, "Transfer function must not be 0.");

        if (SimpleTransferFunction* tester = dynamic_cast<SimpleTransferFunction*>(tf)) {
            return new SimpleTransferFunctionEditor(prop, tester);
        }

        if (Geometry1DTransferFunction* tester = dynamic_cast<Geometry1DTransferFunction*>(tf)) {
            return new Geometry1DTransferFunctionEditor(prop, tester);
        }

        if (Geometry2DTransferFunction* tester = dynamic_cast<Geometry2DTransferFunction*>(tf)) {
            return new Geometry2DTransferFunctionEditor(prop, tester);
        }

        return 0;
    }

}
