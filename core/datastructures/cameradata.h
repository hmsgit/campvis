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

#ifndef CAMERADATA_H__
#define CAMERADATA_H__

#include "cgt/camera.h"
#include "core/datastructures/abstractdata.h"
#include <vector>

namespace campvis {
    /**
     * Data object storing camera data.
     */
    class CAMPVIS_CORE_API CameraData : public AbstractData {
    public:
        /**
         * Constructor, creating a new CameraData object initialized by \a camera.
         * \param   camera  Camera object used for initialization (optional)
         */
        explicit CameraData(const cgt::Camera& camera = cgt::Camera());

        /**
         * Destructor.
         */
        virtual ~CameraData();


        /// \see AbstractData::clone()
        virtual CameraData* clone() const;

        /// \see AbstractData::getLocalMemoryFootprint()
        virtual size_t getLocalMemoryFootprint() const;

        /// \see AbstractData::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;

        /// \see AbstractData::getTypeAsString()
        virtual std::string getTypeAsString() const;


        /**
         * Const getter for the camera settings.
         * \return  _camera
         */
        const cgt::Camera& getCamera() const;

        /**
         * Non-const getter for the camera settings.
         * \return  _camera
         */
        cgt::Camera& getCamera();

        /**
         * Sets the camera settings to \a camera.
         * \param   camera  New camera settings.
         */
        void setCamera(const cgt::Camera& camera);

    protected:
        cgt::Camera _camera;        ///< The cgt::Camera object storing the camera setup

    };

}

#endif // CAMERADATA_H__
