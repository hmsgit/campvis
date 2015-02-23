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

#ifndef GLGAUSSIANFILTER_H__
#define GLGAUSSIANFILTER_H__

#include <string>

#include "core/pipeline/abstractprocessordecorator.h"
#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/optionproperty.h"

#include "modules/modulesapi.h"
#include "modules/processorfactory.h"

namespace cgt {
    class BufferObject;
    class Shader;
}

namespace campvis {
    /**
     * Performs a gaussian filtering on the input image using OpenGL.
     */
    class CAMPVIS_MODULES_API GlGaussianFilter : public VisualizationProcessor {
    public:
        /**
         * Constructs a new GlGaussianFilter Processor
         **/
        GlGaussianFilter(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~GlGaussianFilter();

        /// \see AbstractProcessor::init
        virtual void init();
        /// \see AbstractProcessor::deinit
        virtual void deinit();
        
        /** 
         * To be used in ProcessorFactory static methods
         */
        static const std::string getId() { return "GlGaussianFilter"; };
        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return getId(); };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Performs a gaussian filtering on the input image using OpenGL."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };

        DataNameProperty p_inputImage;      ///< ID for input volume
        DataNameProperty p_outputImage;     ///< ID for output gradient volume

        FloatProperty p_sigma;              ///< Sigma for specifying kernel size

    protected:
        /// \see AbstractProcessor::updateResult
        virtual void updateResult(DataContainer& dataContainer);
        /// \see AbstractProcessor::updateProperties
        virtual void updateProperties(DataContainer& dataContainer);

        /**
         * Applys the morphology filter \a filter to \a inputImage.
         * \param   inputTexture    Input image for the filter
         * \param   filter          Filter to apply (should be _erosionFilter or _dilationFilter or compatible)
         * \return  An OpenGL texture with the filtered image
         */
        cgt::Texture* applyFilter(const cgt::Texture* inputTexture, cgt::Shader* filter) const;


        cgt::Shader* _shader2D;             ///< Shader for performing 2D Gaussian blur
        cgt::Shader* _shader3D;             ///< Shader for performing 3D Gaussian blur
        cgt::BufferObject* _kernelBuffer;   ///< Texture Buffer ID storing the kernel
        GLuint _kernelBufferTexture;

        static const std::string loggerCat_;
    };

    // Instantiate template to register the pipelines.
    template class SmartProcessorRegistrar<GlGaussianFilter>;
}

#endif // GLGAUSSIANFILTER_H__
