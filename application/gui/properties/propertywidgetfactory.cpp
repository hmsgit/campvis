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

#include "propertywidgetfactory.h"

#include "application/gui/properties/abstractpropertywidget.h"
#include "application/gui/properties/buttonpropertywidget.h"
#include "application/gui/properties/boolpropertywidget.h"
#include "application/gui/properties/camerapropertywidget.h"
#include "application/gui/properties/datanamepropertywidget.h"
#include "application/gui/properties/intpropertywidget.h"
#include "application/gui/properties/floatpropertywidget.h"
#include "application/gui/properties/metapropertywidget.h"
#include "application/gui/properties/optionpropertywidget.h"
#include "application/gui/properties/stringpropertywidget.h"
#include "application/gui/properties/transferfunctionpropertywidget.h"

#include "core/datastructures/datacontainer.h"
#include "core/properties/abstractproperty.h"
#include "core/properties/cameraproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/optionproperty.h"
#include "core/properties/transferfunctionproperty.h"


namespace campvis {

    AbstractPropertyWidget* PropertyWidgetFactory::createWidget(AbstractProperty* property, DataContainer* dc) {
        tgtAssert(property != 0, "Property must not be 0.");

        if (BoolProperty* tester = dynamic_cast<BoolProperty*>(property)) {
            return new BoolPropertyWidget(tester);
        }
        if (ButtonProperty* tester = dynamic_cast<ButtonProperty*>(property)) {
            return new ButtonPropertyWidget(tester);
        }

        // OptionProperty must test before IntProperty
        if (AbstractOptionProperty* tester = dynamic_cast<AbstractOptionProperty*>(property)) {
            return new OptionPropertyWidget(tester);
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

        // DataNameProperty must test before StringProperty
        if (DataNameProperty* tester = dynamic_cast<DataNameProperty*>(property)) {
            return new DataNamePropertyWidget(tester, dc);
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

        if (MetaProperty* tester = dynamic_cast<MetaProperty*>(property)) {
            return new MetaPropertyWidget(tester, dc);
        }

        return 0;
    }

}