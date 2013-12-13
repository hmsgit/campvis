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

#include "cameraproperty.h"

namespace campvis {


    const std::string CameraProperty::loggerCat_ = "CAMPVis.core.datastructures.CameraProperty";

    CameraProperty::CameraProperty(const std::string& name, const std::string& title, tgt::Camera cam /*= tgt::Camera()*/, int invalidationLevel /*= AbstractProcessor::INVALID_RESULT*/)
        : GenericProperty<tgt::Camera>(name, title, cam, invalidationLevel)
    {
    }

    CameraProperty::~CameraProperty() {
    }

    void CameraProperty::setWindowRatio(float r) {
        if (r == 0) {
            LERROR("Window ratio must not be 0.");
            return;
        }
        tgt::Camera c = getValue();
        c.setWindowRatio(r);
        setValue(c);
    }

}
