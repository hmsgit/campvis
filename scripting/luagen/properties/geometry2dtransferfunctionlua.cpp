// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

#include "geometry2dtransferfunctionlua.h"

#include "cgt/assert.h"
#include "cgt/shadermanager.h"
#include "cgt/glcontextmanager.h"
#include "cgt/qt/qtthreadedcanvas.h"

#include "application/gui/qtcolortools.h"
#include "application/gui/properties/tfgeometrymanipulator.h"

#include "core/classification/geometry2dtransferfunction.h"
#include "core/classification/tfgeometry2d.h"
#include "core/datastructures/imagerepresentationlocal.h"
#include "core/properties/transferfunctionproperty.h"

namespace campvis {

    Geometry2DTransferFunctionLua::Geometry2DTransferFunctionLua(TransferFunctionProperty* prop, Geometry2DTransferFunction* tf)
        : AbstractTransferFunctionLua(prop, tf)
    {
    }

    Geometry2DTransferFunctionLua::~Geometry2DTransferFunctionLua() {
    }

    std::string Geometry2DTransferFunctionLua::getLuaScript(std::string propNamePrefix, std::string luaProc) {
        return "";
    }
}
