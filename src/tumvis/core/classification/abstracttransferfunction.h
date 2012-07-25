#ifndef ABSTRACTTRANSFERFUNCTION_H__
#define ABSTRACTTRANSFERFUNCTION_H__

#include "sigslot/sigslot.h"
#include "tgt/texture.h"
#include "tgt/vector.h"
#include "tbb/include/tbb/atomic.h"
#include "tbb/include/tbb/mutex.h"

#include <string>

namespace tgt {
    class Shader;
    class TextureUnit;
}

namespace TUMVis {

    /**
     * Abstract base class for transfer functions.
     * 
     * The granularity of the transfer function is determined by its size which is directly mapped to 
     * the OpenGL texture size. During classification the transfer function is mapped to the intensity 
     * domain.
     * 
     * \note    Transfer function objects are supposed to be thread-safe as follows:
     *          a) Access to non-OpenGL internals is protected by the local mutex.
     *          b) All OpenGL-related methods must be called by a thread with a valid and locked OpenGL
     *             context. Even though other internals might be changed meanwhile, this ensures that
     *             the OpenGL stuff (e.g. the texture) stays valid for this time.
     * 
     * \todo    Check thread-safety, probably the private local lock is probably not the best design.
     */
    class AbstractTransferFunction {
    public:
        /**
         * Creates a new AbstractTransferFunction.
         * \param   size            Size of the transfer function texture
         * \param   intensityDomain Intensity Domain where the transfer function is mapped to during classification
         */
        AbstractTransferFunction(const tgt::svec3& size, const tgt::vec2& intensityDomain = tgt::vec2(0.f, 1.f));

        /**
         * Destructor, make sure to delete the OpenGL texture beforehand by calling deinit() with a valid OpenGL context!
         */
        virtual ~AbstractTransferFunction();

        /**
         * Deletes the OpenGL texture, hence, this methods has to be called from a thread with a valid OpenGL context!
         */
        virtual void deinit();

        /**
         * Returns the dimensionality of the transfer function.
         * \return  The dimensionality of the transfer function.
         */
        virtual size_t getDimensionality() const = 0;

        /**
         * Binds the transfer function OpenGL texture to the given texture and sets up uniforms.
         * \note    Calling thread must have a valid OpenGL context.
         * \param   shader                      Shader used for rendering
         * \param   texUnit                     Texture unit to bind texture to
         * \param   textureUniform              Uniform name to store texture unit number
         * \param   textureParametersUniform    Uniform name to store texture parameters
         */
        void bind(tgt::Shader* shader, const tgt::TextureUnit& texUnit, const std::string& textureUniform = "_tfTex", const std::string& textureParametersUniform = "_tfTextureParameters");

        /**
         * Sets the intensity Domain where the transfer function is mapped to during classification.
         * \param   newDomain   new intensity domain
         */
        void setIntensityDomain(const tgt::vec2& newDomain);

        /// Signal emitted when transfer function has changed.
        sigslot::signal0<> s_Changed;

    protected:
        /**
         * Creates the texture and uploads it to OpenGL.
         * Gets called by bind() with the local mutex already acquired.
         */
        virtual void createTexture() = 0;

        tgt::svec3 _size;               ///< Size of the transfer function texture
        tgt::vec2 _intensityDomain;     ///< Intensity Domain where the transfer function is mapped to during classification
        tgt::Texture* _texture;         ///< OpenGL lookup texture storing the TF
        tbb::atomic<bool> _dirty;       ///< Flag whether the OpenGL texture has to be updated

        mutable tbb::mutex _localMutex; ///< mutex protecting the local members

        static const std::string loggerCat_;

    };

}

#endif // ABSTRACTTRANSFERFUNCTION_H__
