// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

#ifndef MPRRENDERER_H__
#define MPRRENDERER_H__

#include <string>

#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/allproperties.h"

#include "modules/modulesapi.h"

namespace cgt {
    class Shader;
}

namespace campvis {
    class ImageData;

    /**
     * Extracts arbitrary MPRs from images using a single clip plane.
     */
    class CAMPVIS_MODULES_API MprRenderer : public VisualizationProcessor {
    public:
        /**
         * Constructs a new MprRenderer Processor
         **/
        explicit MprRenderer(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~MprRenderer();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();
        
        /** 
         * To be used in ProcessorFactory static methods
         */
        static const std::string getId() { return "MprRenderer"; };
        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return getId(); };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Extracts arbitrary MPRs from images using a single clip plane."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        DataNameProperty p_sourceImageID;               ///< image ID for input image
        DataNameProperty p_targetImageID;               ///< image ID for output image

        DataNameProperty p_camera;

        Vec3Property p_planeNormal;                     ///< Clipping plane normal
        FloatProperty p_planeDistance;                  ///< Clipping plane distance
        FloatProperty p_planeSize;                      ///< Size of clipping plane

        BoolProperty p_use2DProjection;                  ///< Use 3D Rendering instead of 2D
        BoolProperty p_relativeToImageCenter;           ///< Flag whether to construct image plane relative to image center
        
        TransferFunctionProperty p_transferFunction;    ///< Transfer function

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// \see AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer);

        cgt::Shader* _shader;                           ///< Shader for slice rendering

        static const std::string loggerCat_;
    };
}

#endif // MPRRENDERER_H__
