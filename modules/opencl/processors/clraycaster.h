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

#ifndef CLRAYCASTER_H__
#define CLRAYCASTER_H__

#include <string>

#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/cameraproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/floatingpointproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/transferfunctionproperty.h"

#include "kisscl/kisscl.h"

namespace kisscl {
    class Context;
    class Image;
    class GLTexture;
    class Program;
}

namespace campvis {
    /**
     * Proof of concept processor for OpenCL based Volume raycasting.
     */
    class CLRaycaster : public VisualizationProcessor {
    public:
        /**
         * Creates a CLRaycaster.
         * \note    The render target size property of this CLRaycaster will automatically 
         *          be assigned as shared property of the given \a renderTargetSize property.
         * \see     VisualizationProcessor
         * \param   viewportSizeProp            Reference to the parent pipeline's render target size property.
         * \param   fragmentShaderFileName      Filename for the fragment shader being automatically loaded.
         * \param   bindEntryExitDepthTextures  Flag whether to also bind the depth textures of the entry-/exit points.
         */
        CLRaycaster(IVec2Property* viewportSizeProp);

        /**
         * Destructor
         **/
        virtual ~CLRaycaster();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "CLRaycaster"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Proof of concept processor for OpenCL based Volume raycasting."; };
        /// \see AbstractProcessor::getAuthor()
        virtual const std::string getAuthor() const { return "Christian Schulte zu Berge <christian.szb@in.tum.de>"; };
        /// \see AbstractProcessor::getProcessorState()
        virtual ProcessorState getProcessorState() const { return AbstractProcessor::EXPERIMENTAL; };

        /**
         * Initalizes the Processor, loads and builds the shader.
         * \see     AbstractProcessor::init
         * \note    When overwriting this method, make sure to call the base class version first.
         */
        virtual void init();

        /**
         * Disposes the shader and deinitializes the processor.
         * \see     AbstractProcessor::deinit
         * \note    When overwriting this method, make sure to call the base class version at the end.
         */
        virtual void deinit();

        /**
         * Performs sanity checks, sets up the rendering and calls CLRaycaster::processImpl().
         * This method first reads the input image, entry and exit points from \a data and validates them. On sucess
         * the shader will be rebuild if necessary, the shader will be activated, common uniforms will be set and
         * the textures and transfer function will be bound before calling processImpl().
         *
         * \sa      CLRaycaster::processImpl()
         * \param   data    DataContainer to work on.
         */
        virtual void process(DataContainer& data);

        DataNameProperty _sourceImageID;                ///< image ID for input image
        DataNameProperty _entryImageID;                 ///< image ID for output entry points image
        DataNameProperty _exitImageID;                  ///< image ID for output exit points image
        DataNameProperty _targetImageID;                ///< image ID for output image

        CameraProperty _camera;                         ///< Camera used for ray casting
        TransferFunctionProperty _transferFunction;     ///< Transfer function
        FloatProperty _samplingStepSize;                ///< Ray casting step size
        BoolProperty _jitterEntryPoints;                ///< Flag whether to jitter the entry points

    protected:

        /**
         * Returns an additional header that will be linked into the fragment shader.
         * Gets calles when building \a _shader. Overwrite this method to add a processor specific header to
         * your fragment shader.
         * \note    When overwriting this method make sure to call the base class version and append its result.
         * \return  The current default implementation returns an empty string.
         */
        virtual std::string generateHeader() const;

        kisscl::Context* _clContext;
        kisscl::Program* _clProgram;

        kisscl::Image* _imgVolume;
        clock_t _volumeTimestamp;
        kisscl::Image* _imgTf;
        clock_t _tfTimestamp;
        kisscl::GLTexture* _texEntryPointsColor;
        kisscl::GLTexture* _texExitPointsColor;
        kisscl::GLTexture* _texOutColor;



        static const std::string loggerCat_;
    };

}

#endif // CLRAYCASTER_H__
