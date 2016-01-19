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


#include "application/gui/properties/propertywidgetfactory.h"

#include "application/gui/properties/boolpropertywidget.h"
#include "application/gui/properties/buttonpropertywidget.h"
#include "application/gui/properties/colorpropertywidget.h"
#include "application/gui/properties/datanamepropertywidget.h"
#include "application/gui/properties/floatpropertywidget.h"
#include "application/gui/properties/intpropertywidget.h"
#include "application/gui/properties/metapropertywidget.h"
#include "application/gui/properties/optionpropertywidget.h"
#include "application/gui/properties/statuspropertywidget.h"
#include "application/gui/properties/stringpropertywidget.h"
#include "application/gui/properties/transferfunctionpropertywidget.h"

namespace campvis {

    // explicitly instantiate templates to register the property widgets
    template class PropertyWidgetRegistrar<BoolPropertyWidget, BoolProperty>;
    template class PropertyWidgetRegistrar<ButtonPropertyWidget, ButtonProperty>;
    template class PropertyWidgetRegistrar<ColorPropertyWidget, ColorProperty>;

    template class PropertyWidgetRegistrar<DataNamePropertyWidget, DataNameProperty, 10>;

    template class PropertyWidgetRegistrar<FloatPropertyWidget, FloatProperty>;
    template class PropertyWidgetRegistrar<Vec2PropertyWidget, typename VecPropertyWidgetTraits<2>::PropertyType, 5>;
    template class PropertyWidgetRegistrar<Vec3PropertyWidget, typename VecPropertyWidgetTraits<3>::PropertyType, 5>;
    template class PropertyWidgetRegistrar<Vec4PropertyWidget, typename VecPropertyWidgetTraits<4>::PropertyType, 5>;

    template class PropertyWidgetRegistrar<IntPropertyWidget, IntProperty>;
    template class PropertyWidgetRegistrar<IVec2PropertyWidget, typename IVecPropertyWidgetTraits<2>::PropertyType, 5>;
    template class PropertyWidgetRegistrar<IVec3PropertyWidget, typename IVecPropertyWidgetTraits<3>::PropertyType, 5>;
    template class PropertyWidgetRegistrar<IVec4PropertyWidget, typename IVecPropertyWidgetTraits<4>::PropertyType, 5>;

    template class PropertyWidgetRegistrar<MetaPropertyWidget, MetaProperty>;
    template class PropertyWidgetRegistrar<OptionPropertyWidget, AbstractOptionProperty, 10>;
    template class PropertyWidgetRegistrar<StatusPropertyWidget, StatusProperty>;
    template class PropertyWidgetRegistrar<StringPropertyWidget, StringProperty>;
    template class PropertyWidgetRegistrar<TransferFunctionPropertyWidget, TransferFunctionProperty>;
}