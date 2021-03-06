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

#ifndef IPSVIRAYCASTER_H__
#define IPSVIRAYCASTER_H__

#include "core/pipeline/raycastingprocessor.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/transferfunctionproperty.h"

#include "modules/modulesapi.h"

#include <string>

namespace cgt {
    class Shader;
    class Texture;
}

namespace campvis {
    class CameraData;
    class LightSourceData;
    class VoxelHierarchyMapper;

    /**
     * Raycaster that implements the Image Plane Sweep Volume Illumation algorithm of Sund�n et al.
     * This raycasting processor supports real-time directional light shadowing and additionally
     * uses the VoxelHierarchyMapper for optimized entry-exit points.
     * 
     * Requires OpenGL 4.4!
     */
    class CAMPVIS_MODULES_API IpsviRaycaster : public RaycastingProcessor {
    public:
        enum AdditionalInvalidationLevels {
            INVALID_BBV = AbstractProcessor::FIRST_FREE_TO_USE_INVALIDATION_LEVEL,
            INVALID_IC_TEXTURES = AbstractProcessor::FIRST_FREE_TO_USE_INVALIDATION_LEVEL << 1,
        };

        /**
         * Constructs a new IpsviRaycaster Processor
         **/
        explicit IpsviRaycaster(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~IpsviRaycaster();
        
        /** 
         * To be used in ProcessorFactory static methods
         */
        static const std::string getId() { return "IpsviRaycaster"; };
        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return getId(); };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Raycaster that implements the Image Plane Sweep Volume Illumation algorithm of Sund�n et al."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };

        /// \see AbstractProcessor::init
        virtual void init();
        /// \see AbstractProcessor::deinit
        virtual void deinit();

        DataNameProperty p_lightId;                 ///< Name/ID for the LightSource to use
        IntProperty p_sweepLineWidth;               ///< Width of the sweep line in pixels
        IVec2Property p_icTextureSize;              ///< Size of the Illumination Cache texture
        FloatProperty p_shadowIntensity;            ///< Intensity of the shadowing effect
    
    protected:
        /// \see RaycastingProcessor::updateResult()
        virtual void updateResult(DataContainer& data) override;
        /// \see RaycastingProcessor::processImpl()
        virtual void processImpl(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image);

        void processDirectional(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image, const CameraData& camera, const LightSourceData& light);
        void processPointLight(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image, const CameraData& camera, const LightSourceData& light);

        VoxelHierarchyMapper* _vhm;     ///< for optimizing entry/exit points
        cgt::Texture* _icTextures[2];   ///< Illumination cache textures
        bool _gl44Supported;            ///< Flag whether OpenGL 4.4 is supported.

        static const std::string loggerCat_;
    };
}

#endif // IPSVIRAYCASTER_H__
