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
        validate(INVALID_RESULT);
    }
}