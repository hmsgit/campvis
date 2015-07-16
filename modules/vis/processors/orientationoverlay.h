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

#ifndef ORIENTATIONOVERLAY_H__
#define ORIENTATIONOVERLAY_H__

#include <string>

#include "cgt/matrix.h"

#include "core/pipeline/abstractprocessordecorator.h"
#include "core/pipeline/processordecoratorbackground.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/optionproperty.h"

#include "modules/modulesapi.h"

namespace cgt {
    class Shader;
    class Texture;
}

namespace campvis {
    class ImageData;
    class MeshGeometry;

    /**
     * Performs the composition of a multiple render targets
     */
    class CAMPVIS_MODULES_API OrientationOverlay : public VisualizationProcessor {
    public:
        /**
         * Constructs a new OrientationOverlay Processor
         **/
        explicit OrientationOverlay(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~OrientationOverlay();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();
        
        /** 
         * To be used in ProcessorFactory static methods
         */
        static const std::string getId() { return "OrientationOverlay"; };
        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return getId(); };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Renders a small cube to indicate the orientation of a data set with respect to the world coordinate system."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };

        DataNameProperty p_camera;                  ///< data ID for the camera data
        DataNameProperty p_sourceImageId;           ///< image ID of the image to indicate orientation for
        DataNameProperty p_targetImageId;           ///< image ID for rendered output image
        DataNameProperty p_passThroughImageId;      ///< image ID for for optional passthrough image

        FloatProperty p_cubeSize;
        Vec2Property p_cubePosition;
        BoolProperty p_enableTexturing;
        GenericProperty<cgt::mat4> p_flipMatrix;    ///< additional flip matrix to align the orientation overlay (optional)

    protected:
        virtual void updateResult(DataContainer& dataContainer);
        virtual void updateProperties(DataContainer& dataContainer);

        void createCube();

        MeshGeometry* _cubeGeometry;            ///< MeshGeometry used for rendering the cube
        cgt::Shader* _shader;                   ///< Shader for rendering
        cgt::Shader* _passthroughShader;        ///< Shader for rendering

        cgt::Texture* _textures;                ///< 2D array Texture for faces (order: front, back, top, bottom, left, right)

        static const std::string loggerCat_;
    };
}

#endif // ORIENTATIONOVERLAY_H__
