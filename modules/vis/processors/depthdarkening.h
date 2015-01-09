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

#ifndef DEPTHDARKENING_H__
#define DEPTHDARKENING_H__

#include <string>

#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"

#include "modules/modulesapi.h"

namespace cgt {
    class Shader;
}

namespace campvis {
    class ImageData;
    class GlReduction;

    /**
     * Extracts a slice from a 3D image and renders it into a rendertarget.
     */
    class CAMPVIS_MODULES_API DepthDarkening : public VisualizationProcessor {
    public:
        /**
         * Constructs a new DepthDarkening Processor
         **/
        DepthDarkening(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~DepthDarkening();

        /// \see AbstractProcessor::init
        virtual void init();

        /// \see AbstractProcessor::deinit
        virtual void deinit();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "DepthDarkening"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Applies depth darkening post processing to simulate shadows."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };

        DataNameProperty p_inputImage;               ///< image ID for input image
        DataNameProperty p_outputImage;              ///< image ID for output image

        FloatProperty p_sigma;                       ///< sigma, standard deviation of the gaussian filter
        FloatProperty p_lambda;                      ///< strength of depth effect

        BoolProperty p_useColorCoding;               ///< Flag whether to use depth color coding
        Vec3Property p_coldColor;                    ///< Cold color (color for far objects)
        Vec3Property p_warmColor;                    ///< Warm color (color for near objects)

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// \see    AbstractProcessor::updateShader
        virtual void updateShader();

        /**
         * Generates the GLSL Header
         * \return  #define uSE_COLORCODING if p_useColorCoding is set
         */
        std::string generateHeader() const;

        cgt::Shader* _shader;                           ///< Shader for slice rendering
        GlReduction* _glReduction;

        static const std::string loggerCat_;
    };

}

#endif // DEPTHDARKENING_H__
