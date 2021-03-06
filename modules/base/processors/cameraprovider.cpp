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

#include "cameraprovider.h"
#include "core/datastructures/cameradata.h"

namespace campvis {
    static const GenericOption<cgt::Camera::ProjectionMode> projectionOptions[3] = {
        GenericOption<cgt::Camera::ProjectionMode>("perspective", "Perspective", cgt::Camera::PERSPECTIVE),
        GenericOption<cgt::Camera::ProjectionMode>("orthographic", "Orthographice", cgt::Camera::ORTHOGRAPHIC),
        GenericOption<cgt::Camera::ProjectionMode>("frustum", "Frustum", cgt::Camera::FRUSTUM)
    };

    const std::string CameraProvider::loggerCat_ = "CAMPVis.modules.io.CameraProvider";

    CameraProvider::CameraProvider() 
        : AbstractProcessor()
        , p_cameraId("CameraId", "Camera ID", "camera", DataNameProperty::WRITE)
        , p_position("Position", "Camera Position", cgt::vec3(0.f, 0.f, -100.f), cgt::vec3(-10000.f), cgt::vec3(10000.f))
        , p_focus("Focus", "Camera Focus", cgt::vec3(0.f), cgt::vec3(-10000.f), cgt::vec3(10000.f))
        , p_upVector("UpVector", "Camera Up Vector", cgt::vec3(0.f, 1.f, 0.f), cgt::vec3(-1.f), cgt::vec3(1.f))
        , p_fov("FoV", "Field of View", 45.f, 1.f, 180.f)
        , p_aspectRatio("AspectRatio", "Aspect Ratio", 1.f, .001f, 100.f)
        , p_clippingPlanes("ClippingPlanes", "Near/Far Clipping Planes", cgt::vec2(.1f, 1000.f), cgt::vec2(0.f), cgt::vec2(10000.f))
        , p_projectionMode("ProjectionMode", "Projection Mode", projectionOptions, 3)
    {
        addProperty(p_cameraId);
        addProperty(p_position);
        addProperty(p_focus);
        addProperty(p_upVector);
        addProperty(p_fov);
        addProperty(p_aspectRatio);
        addProperty(p_clippingPlanes);
        addProperty(p_projectionMode);
    }

    CameraProvider::~CameraProvider() {

    }

    void CameraProvider::updateResult(DataContainer& dataContainer) {
        CameraData* cameraData = new CameraData(cgt::Camera(
            p_position.getValue(),
            p_focus.getValue(),
            p_upVector.getValue(),
            p_fov.getValue(),
            p_aspectRatio.getValue(),
            p_clippingPlanes.getValue().x,
            p_clippingPlanes.getValue().y+.1f,
            p_projectionMode.getOptionValue()));

        dataContainer.addData(p_cameraId.getValue(), cameraData);
    }
}