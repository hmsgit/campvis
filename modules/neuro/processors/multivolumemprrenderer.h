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

#ifndef MULTIVOLUMEMPRRENDERER_H__
#define MULTIVOLUMEMPRRENDERER_H__

#include <string>

#include "core/classification/abstracttransferfunction.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/pipeline/abstractprocessordecorator.h"
#include "core/properties/allproperties.h"

#include "modules/modulesapi.h"
#include "modules/vis/tools/voxelhierarchymapper.h"

namespace cgt {
    class Shader;
}

namespace campvis {
    class CameraData;
    class ImageRepresentationGL;
    class ImageSeries;
    class LightSourceData;
    class RenderData;

namespace neuro {

    /**
     * Performs an MPR rendering of multiple images at the same time.
     */
    class CAMPVIS_MODULES_API MultiVolumeMprRenderer : public VisualizationProcessor {
    public:
        /**
         * Constructs a new MultiVolumeMprRenderer Processor
         **/
        MultiVolumeMprRenderer(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~MultiVolumeMprRenderer();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "MultiVolumeMprRenderer"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Performs an MPR rendering of multiple images at the same time."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };

        DataNameProperty p_sourceImage1;        ///< ID for first input images
        DataNameProperty p_sourceImage2;        ///< ID for second input images
        DataNameProperty p_sourceImage3;        ///< ID for third input images
        DataNameProperty p_camera;              ///< input camra

        DataNameProperty p_outputImageId;       ///< ID for output image

        TransferFunctionProperty p_transferFunction1;    ///< Transfer function for first image
        TransferFunctionProperty p_transferFunction2;    ///< Transfer function for second image
        TransferFunctionProperty p_transferFunction3;    ///< Transfer function for third image

        Vec3Property p_planeNormal;                     ///< Clipping plane normal
        FloatProperty p_planeDistance;                  ///< Clipping plane distance
        FloatProperty p_planeSize;                      ///< Size of clipping plane

        BoolProperty p_use2DProjection;                  ///< Use 3D Rendering instead of 2D
        BoolProperty p_relativeToImageCenter;           ///< Flag whether to construct image plane relative to image center


    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// \see AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer);
        /// \see    AbstractProcessor::updateShader
        virtual void updateShader();

        /**
         * \see RaycastingProcessor::generateHeader()
         * \return  "#define APPLY_MASK 1" if \a _applyMask is set to true.
         */
        virtual std::string generateHeader() const;

        cgt::Shader* _shader;                 ///< Shader for rendering

    private:

        static const std::string loggerCat_;
    };

}
}

#endif // MULTIVOLUMEMPRRENDERER_H__
