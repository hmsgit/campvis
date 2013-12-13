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

#ifndef CAMERAPROPERTY_H__
#define CAMERAPROPERTY_H__

#include "tgt/camera.h"
#include "core/properties/genericproperty.h"

namespace campvis {

    class CameraProperty : public GenericProperty<tgt::Camera> {
    public:
        /**
         * Creates a new CameraProperty
         * \param name      Property name (unchangable!)
         * \param title     Property title (e.g. used for GUI)
         * \param cam       Camera to initialize the property with
         * \param invalidationLevel  Invalidation level that this property triggers
         */
        CameraProperty(const std::string& name, const std::string& title, tgt::Camera cam = tgt::Camera(), int invalidationLevel = AbstractProcessor::INVALID_RESULT);

        /**
         * Virtual Destructor
         **/
        virtual ~CameraProperty();


        /**
         * Sets the camera's window ratio.
         * \param r     The new camera's window ratio (viewport.x / viewport.y)
         */
        void setWindowRatio(float r);

    protected:

        static const std::string loggerCat_;
    };

}

#endif // CAMERAPROPERTY_H__
