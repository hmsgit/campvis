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

#include "cameradata.h"

namespace campvis {


    CameraData::CameraData(const cgt::Camera& camera /*= cgt::Camera()*/) 
        : AbstractData()
        , _camera(camera)
    {

    }

    CameraData::~CameraData() {

    }

    CameraData* CameraData::clone() const {
        return new CameraData(*this);
    }

    size_t CameraData::getLocalMemoryFootprint() const {
        return sizeof(*this);
    }

    size_t CameraData::getVideoMemoryFootprint() const {
        return 0;
    }

    const cgt::Camera& CameraData::getCamera() const {
        return _camera;
    }

    cgt::Camera& CameraData::getCamera() {
        return _camera;
    }

    void CameraData::setCamera(const cgt::Camera& camera) {
        _camera = camera;
    }

    std::string CameraData::getTypeAsString() const {
        return "Camera Data";
    }

}