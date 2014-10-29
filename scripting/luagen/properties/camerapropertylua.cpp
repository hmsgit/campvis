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

#include "camerapropertylua.h"

#include "core/tools/stringutils.h"

namespace campvis {
    //CameraPropertyLua::CameraPropertyLua(CameraProperty* property, DataContainer* dataContainer)
    //    : AbstractPropertyLua(property, true, dataContainer)
    //{
    //}

    CameraPropertyLua::~CameraPropertyLua() {
    }

    //void CameraPropertyLua::updateLuaFromProperty() {
    //    CameraProperty* prop = static_cast<CameraProperty*>(_property);
    //    _lblCameraPosition->setText("Position: " + QString::fromStdString(StringUtils::toString(prop->getValue().getPosition())));
    //    _lblFocusPosition->setText("Focus: " + QString::fromStdString(StringUtils::toString(prop->getValue().getFocus())));
    //    _lblLookDirection->setText("Look Direction: " + QString::fromStdString(StringUtils::toString(prop->getValue().getLook())));
    //    _lblUpVector->setText("Up Vector: " + QString::fromStdString(StringUtils::toString(prop->getValue().getUpVector())));
    //}

    std::string CameraPropertyLua::getLuaScript(std::string prefix) {
        std::string ret = "";
    //    cgt::Camera cam =  static_cast<CameraProperty*>(_property)->getValue();
    //    cgt::vec3 pos = cam.getPosition();
    //    cgt::vec3 focus = cam.getFocus();
    //    cgt::vec3 up = cam.getUpVector();
    //    float fovy = cam.getFovy();
    //    float ratio = cam.getRatio();
    //    float distn = cam.getNearDist();
    //    float distf = cam.getFarDist();
    //    int pm = cam.getProjectionMode();

    //    ret += "getNestedProperty(\"" + prefix+ _property->getName() + "\"):setValue(cgt.Camera("
    //        + "cgt.vec3(" + StringUtils::toString(pos.x) + "," + StringUtils::toString(pos.y) + "," + StringUtils::toString(pos.z) + ")"
    //        + "," + "cgt.vec3(" + StringUtils::toString(focus.x) + "," + StringUtils::toString(focus.y) + "," + StringUtils::toString(focus.z) + ")"
    //        + "," + "cgt.vec3(" + StringUtils::toString(up.x) + "," + StringUtils::toString(up.y) + "," + StringUtils::toString(up.z) + ")"
    //        + "," + StringUtils::toString(fovy)
    //        + "," + StringUtils::toString(ratio)
    //        + "," + StringUtils::toString(distn)
    //        + "," + StringUtils::toString(distf)
    //        + "," + StringUtils::toString(pm)
    //        +"))";

    //    //ret = "-- If required need to implement lua interface for cgt:Camera";
        return ret;
    }

}