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

#ifndef VISUALIZATIONPROCESSOR_H__
#define VISUALIZATIONPROCESSOR_H__

#include "cgt/framebufferobject.h"
#include "cgt/texture.h"
#include "cgt/vector.h"

#include "core/pipeline/abstractprocessor.h"
#include "core/properties/numericproperty.h"

namespace campvis {
    class ImageData;

    /**
     * Specialization of AbstractProcessor for visualization purposes.
     * 
     * VisualizationProcessors are required to be called by a VisualizationPipeline which ensure
     * to provide a valid OpenGL context when calling the processor's process() method. Hence, a
     * VisualizationProcessor is allowed/capable of performing OpenGl operations.
     * 
     * Each VisualizationProcessor has its own OpenGL FramebufferObject, which is created during
     * init(). For determining the canvas/viewport size, a VisualizationProcessor gets a reference 
     * to the parent pipeline's render target size property during instantiation.
     * 
     * \sa VisualizationPipeline
     */
    class CAMPVIS_CORE_API VisualizationProcessor : public AbstractProcessor {
    public:
        /**
         * Utility struct to encapsulate off-screen rendering with this processor using FBOs.
         * This guard essentially activates the VisualizationProcessor's FBO and sets the OpenGL
         * viewport size. Upon destruction it detaches all textures and deactivates the FBO again.
         */
        struct CAMPVIS_CORE_API FramebufferActivationGuard {
        public:
            /**
             * Constructor.
             * Activates the FBO and sets the OpenGL viewport size to the effective viewport size of \a vp.
             * \param   vp  This Visualization processor (usually pointer to this).
             */
            explicit FramebufferActivationGuard(VisualizationProcessor* vp)
                : _parentProcessor(vp)
                , _fbo(vp->_fbo)
            {
                cgtAssert(_fbo != nullptr, "FBO must not be 0.");
                cgtAssert(_fbo->getId() != 0, "The FBO's OpenGL ID is 0, this is wrong.");
                _fbo->activate();

                const cgt::ivec2& windowSize = vp->getEffectiveViewportSize();
                glViewport(0, 0, static_cast<GLsizei>(windowSize.x), static_cast<GLsizei>(windowSize.y));
            }

            /**
             * Destructor, detaches all textures and deactivates the FBO.
             */
            ~FramebufferActivationGuard() {
                _fbo->detachAll();
                _fbo->deactivate();
            }


        private:
            VisualizationProcessor* _parentProcessor;
            cgt::FramebufferObject* _fbo;
        };

        /**
         * Creates a VisualizationProcessor.
         * 
         * \note    The render target size property of this VisualizationProcessor will automatically 
         *          be assigned as shared property of the given \a renderTargetSize property.
         * \note    This processor will keep and access \a renderTargetSize, so make sure the referenced
         *          property exists at least as long as this processor or you set it to a different
         *          property before using setViewportSizeProperty().
         *          
         * \param   viewportSizeProp    Pointer to the property defining the viewport size, must not be 0.
         */
        explicit VisualizationProcessor(IVec2Property* viewportSizeProp);

        /**
         * Virtual Destructor
         **/
        virtual ~VisualizationProcessor();

                
        /// \see AbstractProcessor::init()
        virtual void init();
        
        /// \see AbstractProcessor::deinit()
        virtual void deinit();


        /**
         * Sets the property defining the viewport size to \a viewportSizeProp.
         * \note    This processor will keep and access this pointer, so make sure the referenced
         *          property exists at least as long as this processor or you set it to a different
         *          property before.
         * \param   viewportSizeProp    Pointer to the property defining the viewport size, must not be 0.
         */
        virtual void setViewportSizeProperty(IVec2Property* viewportSizeProp);
        
        /**
         * Returns the effective viewport size considering LQ mode.
         * \return  lqMode ? _viewportSize/2 : _viewportSize
         */
        cgt::ivec2 getEffectiveViewportSize() const;

        BoolProperty p_lqMode;                      ///< Flag whether to enable LQ mode (halfsamples effective viewport size)

    protected:

        /**
         * Creates a texture with the given format and attaches it to the FBO to \a attachment.
         * \param   internalFormat  Internal OpenGL texture format
         * \param   attachment      Target attachment for texture
         */
        void createAndAttachTexture(GLint internalFormat, GLenum attachment);

        /**
         * Creates a texture with the given format and attaches it to the FBO using the default attachments.
         * \note    Default attachment are GL_DEPTH_ATTACHMENT for depth textures and 
         *          GL_COLOR_ATTACHMENT_0 + <number of color textures attached>.
         * \param   internalFormat  Internal OpenGL texture format
         */
        void createAndAttachTexture(GLint internalFormat);

        /**
         * Creates a color texture with format GL_RGBA8 and attaches it to the FBO using the 
         * default attachment.
         */
        void createAndAttachColorTexture();

        /**
         * Creates a depth texture with format GL_DEPTH_COMPONENT24 and attaches it to the FBO 
         * using the default attachment.
         */
        void createAndAttachDepthTexture();

        /**
         * Returns the current viewport size as ivec3.
         * \return  cgt::ivec3(getEffectiveViewportSize(), 1)
         */
        cgt::ivec3 getRenderTargetSize() const;


        cgt::FramebufferObject* _fbo;               ///< The FBO used by this VisualizationProcessor
        IVec2Property* _viewportSizeProperty;       ///< Pointer to the property defining the viewport (canvas) size.

        static const std::string loggerCat_;
    };

}

#endif // VISUALIZATIONPROCESSOR_H__
