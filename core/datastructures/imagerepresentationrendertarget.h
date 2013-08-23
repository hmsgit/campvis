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

#ifndef IMAGEREPRESENTATIONRENDERTARGET_H__
#define IMAGEREPRESENTATIONRENDERTARGET_H__

#include "tgt/framebufferobject.h"
#include "tgt/singleton.h"
#include "tgt/texture.h"
#include "tgt/textureunit.h"
#include "tgt/vector.h"
#include "core/datastructures/genericabstractimagerepresentation.h"
#include "core/datastructures/imagerepresentationgl.h"

#include "tbb/atomic.h"
#include "tbb/spin_mutex.h"

#include <list>
#include <utility>
#include <vector>

namespace tgt {
    class Shader;
    class FramebufferObject;
}

namespace campvis {
    class ImageRepresentationGL;
    class ImageRepresentationRenderTarget;
    
    /**
     * Stores render target data.
     * This is basically a wrapper for multiple OpenGL textures (color + depth) and their attachment to the framebuffer.
     * Each ImageRepresentationRenderTarget has at least one color texture and exactly one depth texture attachment.
     * 
     * \note    Its dimensionality must be 2.
     * 
     * \todo    The constructor only asserts if one of the textures or the FBO weren't successfully created.
     *          The member functions don't double check, whether the pointers are != 0. Make sure, that this 
     *          is a sane design and if not, adapt the code.
     */
    class ImageRepresentationRenderTarget : public GenericAbstractImageRepresentation<ImageRepresentationRenderTarget> {
    public:
        /**
         * Creates a new ImageRepresentationRenderTarget representation with one color and one depth attachment
         * and automatically adds it to \a parent which will take ownerwhip.
         *
         * \note    You do \b not own the returned pointer..
         *
         * \param   parent     Image this representation represents, must not be 0, will take ownership of the returned pointer.
         * \param   internalFormatColor   Internal OpenGL format for the color texture.
         * \param   internalFormatDepth   Internal OpenGL format for the depth texture.
         * \return  A pointer to the newly created ImageRepresentationRenderTarget, you do \b not own this pointer!
         */
        static ImageRepresentationRenderTarget* create(ImageData* parent, GLint internalFormatColor = GL_RGBA8, GLint internalFormatDepth = GL_DEPTH_COMPONENT24);

        /**
         * Creates a new ImageRepresentationRenderTarget and a new ImageData with the given specifications.
         * Assigns the representation to the image.
         * 
         * \param   size                  Size of this image (number of elements per dimension)
         * \param   internalFormatColor   Internal OpenGL format for the color texture.
         * \param   internalFormatDepth   Internal OpenGL format for the depth texture.
         * \return  A pair of the created ImageData and the created RenderTarget image representation.
         */
        static std::pair<ImageData*, ImageRepresentationRenderTarget*> createWithImageData(const tgt::svec2& size, GLint internalFormatColor = GL_RGBA8, GLint internalFormatDepth = GL_DEPTH_COMPONENT24);

        /**
         * Creates a new ImageRepresentationRenderTarget representation with one color and one depth attachment
         * and automatically adds it to \a parent which will take ownerwhip.
         *
         * \note    You do \b not own the returned pointer..
         * \note    ImageRepresentationRenderTarget will take ownership of all textures attached to \a fbo.
         *
         * \param   parent      Image this representation represents, must not be 0.
         * \param   fbo         FrameBufferObject to use the attachements from.
         * \return  A pointer to the newly created ImageRepresentationRenderTarget, you do \b not own this pointer!
         */
        static ImageRepresentationRenderTarget* create(ImageData* parent, const tgt::FramebufferObject* fbo);

        /**
         * Creates a new ImageRepresentationRenderTarget and a new ImageData with the given specifications.
         * Assigns the representation to the image.
         * 
         * \note    ImageRepresentationRenderTarget will take ownership of all textures attached to \a fbo.
         *
         * \param   parent      Image this representation represents, must not be 0.
         * \param   fbo         FrameBufferObject to use the attachements from.
         * 
         * \return  A pair of the created ImageData and the created RenderTarget image representation.
         */
        static std::pair<ImageData*, ImageRepresentationRenderTarget*> createWithImageData(const tgt::svec2& size, const tgt::FramebufferObject* fbo);

        /**
         * Destructor
         */
        virtual ~ImageRepresentationRenderTarget();
       
        /**
         * Performs a conversion of \a source to an ImageRepresentationLocal if feasible.
         * Returns 0 if conversion was not successful or source representation type is not compatible.
         * \note    The callee, respectively the callee's parent, has the ownership of the returned pointer.
         * \param   source  Source image representation for conversion.
         * \return  A pointer to a local representation of \a source or 0 on failure. The caller does \b not have ownership.
         */
        static ImageRepresentationRenderTarget* tryConvertFrom(const AbstractImageRepresentation* source);

        /// \see AbstractImageRepresentation::clone()
        virtual ImageRepresentationRenderTarget* clone(ImageData* newParent) const;

        /// \see AbstractImageRepresentation::getLocalMemoryFootprint()
        virtual size_t getLocalMemoryFootprint() const;

        /// \see AbstractImageRepresentation::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;

        /// \see AbstractImageRepresentation::getSubImage
        virtual ImageRepresentationRenderTarget* getSubImage(ImageData* parent, const tgt::svec3& llf, const tgt::svec3& urb) const;

        /**
         * Creates and initializes a new OpenGL texture according to \a internalFormat and attaches it to the FBO.
         * \note    The amout of attachable color textures is limited by the driver/hardware - check 
         *          GpuCaps.getMaxColorAttachments(). You can only attach one depth texture.
         * \param   internalFormat  Internal OpenGL format for the texture to create.
         */
        void createAndAttachTexture(GLint internalFormat);

        /**
         * Gets the number of color textures/attachments of this render target.
         * \return  _colorTextures.size()
         */
        size_t getNumColorTextures() const;

        /**
         * Gets the color texture of this render target
         * \param   index   Index of the color texture attachment to return.
         * \return  _colorTextures[index]
         */
        const tgt::Texture* getColorTexture(size_t index = 0) const;

        /**
         * Gets the depth texture of this render target
         * \return _depthTexture
         */
        const tgt::Texture* getDepthTexture() const;

        /**
         * Activates the render target (binds the FBO and sets the viewport).
         */
        void activate();

        /**
         * Deactivates the render target (unbinds the FBO).
         */
        void deactivate();

        /**
         * Binds the color texture with index \a index of this render target
         * and sets the corresponding shader uniforms.
         * 
         * \param shader                    Shader to set the uniforms to.
         * \param colorTexUnit              Color texture unit.
         * \param colorTexUniform           Name for color texture sampler.
         * \param texParamsUniform           Name for texture parameters struct uniform.
         * \param index                     Index of the color texture to bind.
         */
        void bindColorTexture(
            tgt::Shader* shader,
            const tgt::TextureUnit& colorTexUnit, 
            const std::string& colorTexUniform = "_colorTexture",
            const std::string& texParamsUniform = "_texParams",
            size_t index = 0) const;

        /**
         * Binds the depth texture of this render target and sets the corresponding shader uniforms.
         * 
         * \param shader                    Shader to set the uniforms to.
         * \param depthTexUnit              Depth texture unit.
         * \param depthTexUniform           Name for depth texture sampler.
         * \param texParamsUniform           Name for texture parameters struct uniform.
         */
        void bindDepthTexture(
            tgt::Shader* shader,
            const tgt::TextureUnit& depthTexUnit,
            const std::string& depthTexUniform = "_depthTexture", 
            const std::string& texParamsUniform = "_texParams") const;

        /**
         * Binds the color texture with index \a index and the depth texture of this render target
         * and sets the corresponding shader uniforms.
         * 
         * \param shader                    Shader to set the uniforms to.
         * \param colorTexUnit              Color texture unit.
         * \param depthTexUnit              Depth texture unit.
         * \param colorTexUniform           Name for color texture sampler.
         * \param depthTexUniform           Name for depth texture sampler.
         * \param texParamsUniform           Name for texture parameters struct uniform.
         * \param index                     Index of the color texture to bind.
         */
        void bind(
            tgt::Shader* shader,
            const tgt::TextureUnit& colorTexUnit, 
            const tgt::TextureUnit& depthTexUnit, 
            const std::string& colorTexUniform = "_colorTexture",
            const std::string& depthTexUniform = "_depthTexture",
            const std::string& texParamsUniform = "_texParams",
            size_t index = 0) const;


    protected:
        /**
         * Creates a new ImageRepresentationRenderTarget representation with one color and one depth attachment.
         *
         * \param   parent                Image this representation represents, must not be 0.
         * \param   internalFormatColor   Internal OpenGL format for the color texture.
         * \param   internalFormatDepth   Internal OpenGL format for the depth texture.
         */
        ImageRepresentationRenderTarget(ImageData* parent, GLint internalFormatColor = GL_RGBA8, GLint internalFormatDepth = GL_DEPTH_COMPONENT24);

        /**
         * Creates a new ImageRepresentationRenderTarget from one color texture and one optional depth texture.
         * \param   parent                Image this representation represents, must not be 0.
         * \param   colorTexture    Color texture, must not be 0
         * \param   depthTexture    Depth texture, optional, must have valid internal format and same dimensions as \a colorTexture
         */
        ImageRepresentationRenderTarget(ImageData* parent, const ImageRepresentationGL* colorTexture, const ImageRepresentationGL* depthTexture = 0);

        /**
         * Creates a new ImageRepresentationRenderTarget from one color texture and one optional depth texture.
         * \note    ImageRepresentationRenderTarget will take ownership of all textures attached to \a fbo.
         * \param   parent      Image this representation represents, must not be 0.
         * \param   fbo         FrameBufferObject to use the attachements from.
         */
        ImageRepresentationRenderTarget(ImageData* parent, const tgt::FramebufferObject* fbo);

        /**
         * Activates the texture unit \a texUnit and binds the color texture.
         * \param   texUnit     Texture unit to activate
         */
        void bindColorTexture(const tgt::TextureUnit& texUnit, size_t index = 0) const;

        /**
         * Activates the texture unit \a texUnit and binds the depth texture.
         * \param   texUnit     Texture unit to activate
         */
        void bindDepthTexture(const tgt::TextureUnit& texUnit) const;

        std::vector<tgt::Texture*> _colorTextures;      ///< color textures
        tgt::Texture* _depthTexture;                    ///< depth texture
        tgt::FramebufferObject* _fbo;                   ///< Framebuffer object color and depth textures are attached to


        static const std::string loggerCat_;
    };
}

#endif // IMAGEREPRESENTATIONRENDERTARGET_H__
