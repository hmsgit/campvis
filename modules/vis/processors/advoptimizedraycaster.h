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

#include "modules/modulesapi.h"
#include "modules/vis/tools/voxelhierarchymapper.h"

#include <string>

namespace cgt {
    class Shader;
}

namespace campvis {
    /**
     * Performs ray casting using acceleration data structure. Also, the number of control instructions which is used is reduced to increase the execution performance on the gpu.
     * The method is similar to [THGM11] paper. First, the data to be rendered is voxelized and stored in a 2D texture (each element of the texture is a 32-bit int. So, each element of the texture 
     * can store 32 ). Then, a hierarchy is generated to increase the ray-rendering
     * [THGM11] Sinje Thiedemann, Niklas Henrich, Thorsten Grosch, and Stefan Müller. 2011. Voxel-based global illumination. In Symposium on Interactive 3D Graphics and Games (I3D '11). ACM, New York, NY, USA, 103-110. DOI=10.1145/1944745.1944763 http://doi.acm.org/10.1145/1944745.1944763
     */
    class CAMPVIS_MODULES_API AdvOptimizedRaycaster : public RaycastingProcessor {
    public:
        enum AdditionalInvalidationLevels {
            INVALID_BBV = AbstractProcessor::FIRST_FREE_TO_USE_INVALIDATION_LEVEL
        };

        /**
         * Constructs a new AdvOptimizedRaycaster Processor
         **/
        explicit AdvOptimizedRaycaster(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~AdvOptimizedRaycaster();
        
        /** 
         * To be used in ProcessorFactory static methods
         */
        static const std::string getId() { return "AdvOptimizedRaycaster"; };
        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return getId(); };
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

        BoolProperty p_enableShading;               ///< Flag whether to enable shading
        DataNameProperty p_lightId;                 ///< Name/ID for the LightSource to use

    protected:

        /// \see RaycastingProcessor::processImpl()
        virtual void processImpl(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image);

        /// \see RaycastingProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer);

        /// \see RaycastingProcessor::generateHeader()
        virtual std::string generateHeader() const;
        
        VoxelHierarchyMapper* _vhm;

        static const std::string loggerCat_;
    };
}

#endif // AdvOptimizedRaycaster_H__
