// ================================================================================================
// 
// This file is part of the CAMPVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#ifndef CLRAYCASTER_H__
#define CLRAYCASTER_H__

#include <string>

#include "core/pipeline/visualizationprocessor.h"
#include "core/properties/cameraproperty.h"
#include "core/properties/datanameproperty.h"
#include "core/properties/genericproperty.h"
#include "core/properties/numericproperty.h"
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
         * \param   renderTargetSize            Reference to the parent pipeline's render target size property.
         * \param   fragmentShaderFileName      Filename for the fragment shader being automatically loaded.
         * \param   bindEntryExitDepthTextures  Flag whether to also bind the depth textures of the entry-/exit points.
         */
        CLRaycaster(GenericProperty<tgt::ivec2>& renderTargetSize);

        /**
         * Destructor
         **/
        virtual ~CLRaycaster();

        /// \see AbstractProcessor::getName()
        virtual const std::string getName() const { return "CLRaycaster"; };
        /// \see AbstractProcessor::getDescription()
        virtual const std::string getDescription() const { return "Proof of concept processor for OpenCL based Volume raycasting."; };

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
