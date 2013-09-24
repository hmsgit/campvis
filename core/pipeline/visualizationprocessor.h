// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
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

#ifndef VISUALIZATIONPROCESSOR_H__
#define VISUALIZATIONPROCESSOR_H__

#include "tgt/framebufferobject.h"
#include "tgt/texture.h"
#include "tgt/vector.h"

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
    class VisualizationProcessor : public AbstractProcessor {
    public:
        struct FramebufferActivationGuard {
        public:
            FramebufferActivationGuard(VisualizationProcessor* vp)
                : _parentProcessor(vp)
                , _fbo(vp->_fbo)
            {
                tgtAssert(_fbo != 0, "FBO must not be 0.");
                _fbo->activate();

                const tgt::ivec2& windowSize = vp->_viewportSizeProperty->getValue();
                glViewport(0, 0, static_cast<GLsizei>(windowSize.x), static_cast<GLsizei>(windowSize.y));
            }

            ~FramebufferActivationGuard() {
                _fbo->detachAll();
                _fbo->deactivate();
            }


        private:
            VisualizationProcessor* _parentProcessor;
            tgt::FramebufferObject* _fbo;
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
        VisualizationProcessor(IVec2Property* viewportSizeProp);

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
         * \return  tgt::ivec3(_viewportSizeProperty->getValue(), 1)
         */
        tgt::ivec3 getRenderTargetSize() const;


        tgt::FramebufferObject* _fbo;               ///< The FBO used by this VisualizationProcessor
        IVec2Property* _viewportSizeProperty;       ///< Pointer to the property defining the viewport size.

        static const std::string loggerCat_;
    };

}

#endif // VISUALIZATIONPROCESSOR_H__
