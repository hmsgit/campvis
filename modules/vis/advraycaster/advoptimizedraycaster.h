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

#ifndef AdvOptimizedRaycaster_H__
#define AdvOptimizedRaycaster_H__

#include "core/pipeline/raycastingprocessor.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/transferfunctionproperty.h"
#include "modules/vis/advraycaster/rendervolumeVoxelizing.h"
#include "modules/vis/advraycaster/VoxelTextureMipMapping.h"

#include <string>

namespace tgt {
    class Shader;
}

namespace campvis {
    /**
     * Performs ray casting using acceleration data structure. Also, the number of control instructions which is used is reduced to increase the execution performance on the gpu.
     * The method is similar to [THGM11] paper. First, the data to be rendered is voxelized and stored in a 2D texture (each element of the texture is a 32-bit int. So, each element of the texture 
     * can store 32 ). Then, a hierarchy is generated to increase the ray-rendering
     * [THGM11] Sinje Thiedemann, Niklas Henrich, Thorsten Grosch, and Stefan Müller. 2011. Voxel-based global illumination. In Symposium on Interactive 3D Graphics and Games (I3D '11). ACM, New York, NY, USA, 103-110. DOI=10.1145/1944745.1944763 http://doi.acm.org/10.1145/1944745.1944763
     */
    class AdvOptimizedRaycaster : public RaycastingProcessor {
    public:
        enum AdditionalInvalidationLevels {
            INVALID_BBV = AbstractProcessor::FIRST_FREE_TO_USE_INVALIDATION_LEVEL
        };

        /**
         * Constructs a new AdvOptimizedRaycaster Processor
         **/
        AdvOptimizedRaycaster(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~AdvOptimizedRaycaster();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "AdvOptimizedRaycaster"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Performs advanced ray casting to render sparse and big volumes faster."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Morteza Mostajab <mostajab@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        /// \see AbstractProcessor::init
        virtual void init();
        /// \see AbstractProcessor::deinit
        virtual void deinit();

        BoolProperty p_enableShadowing;
        FloatProperty p_shadowIntensity;
        BoolProperty p_enableIntersectionRefinement;

        BoolProperty p_useEmptySpaceSkipping;
    
    protected:
        /// \see HasProperyCollection::updateProperties()
        virtual void updateProperties();

        /// \see RaycastingProcessor::processImpl()
        virtual void processImpl(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image);

        /// \see RaycastingProcessor::generateHeader()
        virtual std::string generateHeader() const;

        /// Renders the voxelized volume
        void renderVv(DataContainer& dh); 

        VoxelTexMipMapGenerator* _mipMapGen;    //!< Voxel Texture Mip Map Generator
        int _maxMipMapLevel;
        VoxelizedRenderVolume* _vv;             //!< Voxelized Render volume
        tgt::Texture* _vvTex;                   //!< Generated 2D Texture 


        FaceGeometry* _quad;
        tgt::Shader* _voxelGeneratorShdr;

        static const std::string loggerCat_;
    };

}

#endif // AdvOptimizedRaycaster_H__
