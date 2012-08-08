#ifndef IMAGEDATARENDERTARGET_H__
#define IMAGEDATARENDERTARGET_H__

#include "tgt/framebufferobject.h"
#include "tgt/texture.h"
#include "tgt/textureunit.h"
#include "tgt/vector.h"
#include "core/datastructures/imagedata.h"

namespace tgt {
    class Shader;
}

namespace TUMVis {

    /**
     * Stores render target data.
     * This is basically a wrapper for two OpenGL textures (color + depth) and their attachment to the framebuffer.
     * 
     * \note    Its dimensionality must be 2.
     * 
     * \todo    The constructor only asserts if one of the textures or the FBO weren't successfully created.
     *          The member functions don't double check, whether the pointers are != 0. Make sure, that this 
     *          is a sane design and if not, adapt the code.
     */
    class ImageDataRenderTarget : public ImageData {
    public:
        /**
         * Creates a new ImageDataRenderTarget representation.
         *
         * \param size                  Size of this image (number of elements per dimension)
         * \param internalFormatColor   Internal OpenGL format for the color texture.
         * \param internalFormatDepth   Internal OpenGL format for the depth texture.
         */
        ImageDataRenderTarget(const tgt::svec3& size, GLint internalFormatColor = GL_RGBA8, GLint internalFormatDepth = GL_DEPTH_COMPONENT24);

        /**
         * Destructor
         */
        virtual ~ImageDataRenderTarget();
       
        /**
         * \see AbstractData::clone()
         **/
        virtual ImageDataRenderTarget* clone() const;

        /**
         * \see ImageData::getSubImage
         */
        virtual ImageDataRenderTarget* getSubImage(const tgt::svec3& llf, const tgt::svec3& urb) const;


        /**
         * Activates the render target (binds the FBO and sets the viewport).
         */
        void activate();

        /**
         * Deactivates the render target (unbinds the FBO).
         */
        void deactivate();

        /**
         * Binds the color texture without activating a texture unit.
         */
        void bindColorTexture() const;

        /**
         * Binds the depth texture without activating a texture unit.
         */
        void bindDepthTexture() const;

        /**
         * Activates the texture unit \a texUnit and binds the color texture.
         * \param   texUnit     Texture unit to activate
         */
        void bindColorTexture(const tgt::TextureUnit& texUnit) const;

        /**
         * Activates the texture unit \a texUnit and binds the depth texture.
         * \param   texUnit     Texture unit to activate
         */
        void bindDepthTexture(const tgt::TextureUnit& texUnit) const;

        /**
         * Binds the textures of this render target and sets the according shader uniforms.
         * Of \a colorTexUnit or \a depthTexUnit is 0, the corresponding texture will not be bound
         * and the corresponding uniforms will not be set.
         * 
         * \param shader                    Shader to set the uniforms to.
         * \param colorTexUnit              Pointer to color texture unit, may be 0.
         * \param depthTexUnit              Pointer to depth texture unit, may be 0.
         * \param colorTexUniform           Name for color texture unit uniform.
         * \param depthTexUniform           Name for depth texture unit uniform.
         * \param textureParametersUniform  Name for texture parameters uniform.
         */
        void bind(
            tgt::Shader* shader,
            const tgt::TextureUnit* colorTexUnit, 
            const tgt::TextureUnit* depthTexUnit, 
            const std::string& colorTexUniform = "_colorTexture",
            const std::string& depthTexUniform = "_depthTexture",
            const std::string& textureParametersUniform = "_textureParameters") const;


    protected:
        /**
         * Initializes the textures as well as the FBO.
         * \param   internalFormatColor Internal OpenGL format for the color texture.
         * \param   internalFormatDepth Internal OpenGL format for the depth texture.
         */
        void initRenderTarget(GLint internalFormatColor, GLint internalFormatDepth);

        tgt::Texture* _colorTexture;            ///< color texture
        tgt::Texture* _depthTexture;            ///< depth texture
        tgt::FramebufferObject* _fbo;           ///< Framebuffer object coordinating color and depth texture


        static const std::string loggerCat_;
    };

}

#endif // IMAGEDATARENDERTARGET_H__
