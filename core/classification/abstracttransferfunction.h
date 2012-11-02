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

#ifndef ABSTRACTTRANSFERFUNCTION_H__
#define ABSTRACTTRANSFERFUNCTION_H__

#include "sigslot/sigslot.h"
#include "tgt/vector.h"
#include "tbb/include/tbb/atomic.h"
#include "tbb/include/tbb/mutex.h"

#include "core/datastructures/datahandle.h"
#include "core/tools/concurrenthistogram.h"

#include <string>

namespace tgt {
    class Shader;
    class Texture;
    class TextureUnit;
}

namespace campvis {

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
     * \todo    Check thread-safety, the private local lock is probably not the best design.
     */
    class AbstractTransferFunction {
    public:
        typedef ConcurrentGenericHistogramND<float, 1> IntensityHistogramType;

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
         * \param   transFuncUniform            Uniform name to store the TF struct
         */
        void bind(tgt::Shader* shader, const tgt::TextureUnit& texUnit, const std::string& transFuncUniform = "_transferFunction");
        
        /**
         * Creates the OpenGL texture.
         * \note    Calling thread must have a valid OpenGL context.
         */
        void uploadTexture();

        /**
         * Sets the intensity domain where the transfer function is mapped to during classification.
         * \param   newDomain   new intensity domain
         */
        void setIntensityDomain(const tgt::vec2& newDomain);

        /**
         * Returns the intensity domain where the transfer function is mapped to during classification.
         * \return _intensityDomain
         */
        const tgt::vec2& getIntensityDomain() const;

        /**
         * Gets the OpenGL lookup texture storing the TF.
         * \note    Calling thread must have a valid OpenGL context.
         * \return  _texture
         */
        const tgt::Texture* getTexture();

        /**
         * Returns a DataHandle to the image for this transfer function, may be 0.
         * \return  _imageHandle, may be 0!
         */
        DataHandle getImageHandle() const;

        /**
         * Sets the DataHandle for this transfer function, may be 0.
         * \note    This method makes a copy of \a imageHandle, hence does not take ownership.
         * \param   imageHandle     The new DataHandle for this transfer function
         */
        void setImageHandle(const DataHandle& imageHandle);

        /**
         * Returns the intensity histogram
         * \todo    This is NOT thread-safe!
         * \return  _intensityHistogram
         */
        const IntensityHistogramType* getIntensityHistogram() const;

        /// Signal emitted when transfer function has changed.
        sigslot::signal0<> s_changed;

    protected:
        /**
         * Computes the intensity histogram;
         */
        void computeIntensityHistogram() const;

        /**
         * Creates the texture and uploads it to OpenGL.
         * Gets called by bind() with the local mutex already acquired.
         */
        virtual void createTexture() = 0;

        tgt::svec3 _size;                   ///< Size of the transfer function texture
        tgt::vec2 _intensityDomain;         ///< Intensity Domain where the transfer function is mapped to during classification
        tgt::Texture* _texture;             ///< OpenGL lookup texture storing the TF
        tbb::atomic<bool> _dirtyTexture;    ///< Flag whether the OpenGL texture has to be updated

        DataHandle _imageHandle;                                ///< DataHandle to the image for this transfer function. May be 0.
        mutable IntensityHistogramType* _intensityHistogram;    ///< Intensity histogram of the intensity in _imageHandle for the current _intensityDomain
        mutable tbb::atomic<bool> _dirtyHistogram;              ///< Flag whether the intensity histogram has to be updated.

        mutable tbb::mutex _localMutex; ///< mutex protecting the local members

        static const std::string loggerCat_;

    };

}

#endif // ABSTRACTTRANSFERFUNCTION_H__
