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

#include "lightsourceprovider.h"
#include "core/datastructures/lightsourcedata.h"

namespace campvis {
    const std::string LightSourceProvider::loggerCat_ = "CAMPVis.modules.io.LightSourceProvider";

    LightSourceProvider::LightSourceProvider() 
        : AbstractProcessor()
        , p_lightId("LightId", "Light Name/ID", "lightsource", DataNameProperty::WRITE)
        , p_lightPosition("LightPosition", "Light Position", tgt::vec3(-100.f), tgt::vec3(-500.f), tgt::vec3(500.f), tgt::vec3(1.f))
        , p_ambientColor("AmbientColor", "Ambient Light Color", tgt::vec3(0.4f), tgt::vec3(0.f), tgt::vec3(1.f))
        , p_diffuseColor("DiffuseColor", "Diffuse Light Color", tgt::vec3(0.75f), tgt::vec3(0.f), tgt::vec3(1.f))
        , p_specularColor("SpecularColor", "Specular Light Color", tgt::vec3(0.6f), tgt::vec3(0.f), tgt::vec3(1.f))
        , p_shininess("Shininess", "Specular Shininess", 24.f, 0.f, 64.f, 0.5f)
        , p_attenuation("Attenuation", "Attenuation Factors", tgt::vec3(0.f), tgt::vec3(0.f), tgt::vec3(1.f))
    {
        addProperty(p_lightId);
        addProperty(p_lightPosition);
        addProperty(p_ambientColor);
        addProperty(p_diffuseColor);
        addProperty(p_specularColor);
        addProperty(p_shininess);
        addProperty(p_attenuation);
    }

    LightSourceProvider::~LightSourceProvider() {

    }

    void LightSourceProvider::updateResult(DataContainer& data) {
        LightSourceData* lsd = new LightSourceData(
            p_lightPosition.getValue(),
            p_ambientColor.getValue(),
            p_diffuseColor.getValue(),
            p_specularColor.getValue(),
            p_shininess.getValue(),
            p_attenuation.getValue());
        data.addData(p_lightId.getValue(), lsd);

        validate(INVALID_RESULT);
    }

}