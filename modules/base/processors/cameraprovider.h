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

#ifndef CAMERAPROVIDER_H__
#define CAMERAPROVIDER_H__

#include <string>

#include "cgt/camera.h"

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/numericproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/optionproperty.h"

#include "modules/modulesapi.h"

namespace campvis {
    /**
     * Generates CameraData objects.
     */
    class CAMPVIS_MODULES_API CameraProvider : public AbstractProcessor {
    public:
        /**
         * Constructs a new CameraProvider Processor
         **/
        CameraProvider();

        /**
         * Destructor
         **/
        virtual ~CameraProvider();


        /// \see AbstractProcessor::updateResult()
        virtual void updateResult(DataContainer& dataContainer);

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "CameraProvider"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Generates CameraData objects."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::STABLE; };

        DataNameProperty p_cameraId;        ///< Name/ID for the CameraData object

        Vec3Property p_position;            /// location of the camera
        Vec3Property p_focus;               /// location, the camera looks at
        Vec3Property p_upVector;            /// up vector, always normalized

        FloatProperty p_fov;                ///< Field of View
        FloatProperty p_aspectRatio;        ///< Aspect Ratio
        Vec2Property p_clippingPlanes;      ///< Near and far clipping planes

        GenericOptionProperty<cgt::Camera::ProjectionMode> p_projectionMode;   ///< Projection mode

    protected:

        static const std::string loggerCat_;
    };

}

#endif // CAMERAPROVIDER_H__