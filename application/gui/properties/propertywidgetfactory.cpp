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

#include "propertywidgetfactory.h"

#include "application/gui/properties/abstractpropertywidget.h"
#include "application/gui/properties/boolpropertywidget.h"
#include "application/gui/properties/camerapropertywidget.h"
#include "application/gui/properties/intpropertywidget.h"
#include "application/gui/properties/floatpropertywidget.h"
#include "application/gui/properties/stringpropertywidget.h"
#include "application/gui/properties/transferfunctionpropertywidget.h"

#include "core/properties/abstractproperty.h"
#include "core/properties/cameraproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/transferfunctionproperty.h"


namespace campvis {

    AbstractPropertyWidget* PropertyWidgetFactory::createWidget(AbstractProperty* property) {
        tgtAssert(property != 0, "Property must not be 0.");

        if (BoolProperty* tester = dynamic_cast<BoolProperty*>(property)) {
            return new BoolPropertyWidget(tester);
        }

        if (IntProperty* tester = dynamic_cast<IntProperty*>(property)) {
            return new IntPropertyWidget(tester);
        }
        if (IVec2Property* tester = dynamic_cast<IVec2Property*>(property)) {
            return new IVec2PropertyWidget(tester);
        }
        if (IVec3Property* tester = dynamic_cast<IVec3Property*>(property)) {
            return new IVec3PropertyWidget(tester);
        }
        if (IVec4Property* tester = dynamic_cast<IVec4Property*>(property)) {
            return new IVec4PropertyWidget(tester);
        }

        if (FloatProperty* tester = dynamic_cast<FloatProperty*>(property)) {
            return new FloatPropertyWidget(tester);
        }
        if (Vec2Property* tester = dynamic_cast<Vec2Property*>(property)) {
            return new Vec2PropertyWidget(tester);
        }
        if (Vec3Property* tester = dynamic_cast<Vec3Property*>(property)) {
            return new Vec3PropertyWidget(tester);
        }
        if (Vec4Property* tester = dynamic_cast<Vec4Property*>(property)) {
            return new Vec4PropertyWidget(tester);
        }

        if (StringProperty* tester = dynamic_cast<StringProperty*>(property)) {
            return new StringPropertyWidget(tester);
        }

        if (CameraProperty* tester = dynamic_cast<CameraProperty*>(property)) {
            return new CameraPropertyWidget(tester);
        }

        if (TransferFunctionProperty* tester = dynamic_cast<TransferFunctionProperty*>(property)) {
            return new TransferFunctionPropertyWidget(tester);
        }

        return 0;
    }

}