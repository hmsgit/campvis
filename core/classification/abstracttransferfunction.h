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

#ifndef ABSTRACTTRANSFERFUNCTION_H__
#define ABSTRACTTRANSFERFUNCTION_H__

#include "sigslot/sigslot.h"
#include "cgt/vector.h"
#include <tbb/atomic.h>
#include <tbb/mutex.h>

#include "core/coreapi.h"
#include "core/datastructures/datahandle.h"
#include "core/tools/concurrenthistogram.h"

#include <string>

namespace cgt {
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
     */
    class CAMPVIS_CORE_API AbstractTransferFunction {
    public:
        typedef ConcurrentGenericHistogramND<float, 1> IntensityHistogramType;

        /**
         * Creates a new AbstractTransferFunction.
         * \param   size            Size of the transfer function texture
         * \param   intensityDomain Intensity Domain where the transfer function is mapped to during classification
         */
        AbstractTransferFunction(const cgt::svec3& size, const cgt::vec2& intensityDomain = cgt::vec2(0.f, 1.f));

        /**
         * Destructor, make sure to delete the OpenGL texture beforehand by calling deinit() with a valid OpenGL context!
         */
        virtual ~AbstractTransferFunction();

        /**
         * Clones this transfer function.
         * \return  A clone of this transfer function, caller takes ownership of the pointer.
         */
        virtual AbstractTransferFunction* clone() const = 0;

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
         * Returns the intensity domain where this TF has it's non-transparent parts.
         * I.e. the minimum and the maximum intensity being opaque.
         * \return  The intensity domain where this TF has it's non-transparent parts.
         */
        virtual cgt::vec2 getVisibilityDomain() const = 0;

        /**
         * Binds the transfer function OpenGL texture to the given texture and sets up uniforms.
         * \note    Calling thread must have a valid OpenGL context.
         * \param   shader                      Shader used for rendering
         * \param   texUnit                     Texture unit to bind texture to
         * \param   transFuncUniform            Uniform name to store the TF sampler
         * \param   transFuncParamsUniform      Uniform name to store the TF parameters struct
         */
        void bind(cgt::Shader* shader, const cgt::TextureUnit& texUnit, const std::string& transFuncUniform = "_transferFunction", const std::string& transFuncParamsUniform = "_transferFunctionParams");
        
        /**
         * Sets the intensity domain where the transfer function is mapped to during classification.
         * \param   newDomain   new intensity domain
         */
        void setIntensityDomain(const cgt::vec2& newDomain);

        /**
         * Returns the intensity domain where the transfer function is mapped to during classification.
         * \return _intensityDomain
         */
        const cgt::vec2& getIntensityDomain() const;

        /**
         * Gets the OpenGL lookup texture storing the TF.
         * \note    Calling thread must have a valid OpenGL context.
         * \return  _texture
         */
        const cgt::Texture* getTexture();
        
        /// Signal emitted when transfer function has changed.
        sigslot::signal0 s_changed;
        /// Signal emitted when the intensity domain has changed
        sigslot::signal0 s_intensityDomainChanged;

        const cgt::svec3& getSize();
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

        cgt::svec3 _size;                   ///< Size of the transfer function texture
        cgt::vec2 _intensityDomain;         ///< Intensity Domain where the transfer function is mapped to during classification
        cgt::Texture* _texture;             ///< OpenGL lookup texture storing the TF
        tbb::atomic<bool> _dirtyTexture;    ///< Flag whether the OpenGL texture has to be updated

        mutable tbb::mutex _localMutex;     ///< mutex protecting the local members

        static const std::string loggerCat_;

    };

}

#endif // ABSTRACTTRANSFERFUNCTION_H__
