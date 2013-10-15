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

#include "transferfunctioneditorfactory.h"

#include "application/gui/properties/abstracttransferfunctioneditor.h"
#include "application/gui/properties/geometry1dtransferfunctioneditor.h"
#include "application/gui/properties/geometry2dtransferfunctioneditor.h"
#include "application/gui/properties/simpletransferfunctioneditor.h"

#include "core/classification/abstracttransferfunction.h"
#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/geometry2dtransferfunction.h"
#include "core/classification/simpletransferfunction.h"

namespace campvis {

    AbstractTransferFunctionEditor* TransferFunctionEditorFactory::createEditor(AbstractTransferFunction* tf) {
        tgtAssert(tf != 0, "Transfer function must not be 0.");

        if (SimpleTransferFunction* tester = dynamic_cast<SimpleTransferFunction*>(tf)) {
            return new SimpleTransferFunctionEditor(tester);
        }

        if (Geometry1DTransferFunction* tester = dynamic_cast<Geometry1DTransferFunction*>(tf)) {
            return new Geometry1DTransferFunctionEditor(tester);
        }

        if (Geometry2DTransferFunction* tester = dynamic_cast<Geometry2DTransferFunction*>(tf)) {
            return new Geometry2DTransferFunctionEditor(tester);
        }

        return 0;
    }

}
