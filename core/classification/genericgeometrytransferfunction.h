// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#ifndef GENERICGEOMETRYTRANSFERFUNCTION_H__
#define GENERICGEOMETRYTRANSFERFUNCTION_H__

#include "core/classification/abstracttransferfunction.h"

#include "cgt/assert.h"
#include "cgt/framebufferobject.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/texture.h"
#include "cgt/textureunit.h"

#include <vector>

namespace campvis {

    /**
     * Generic base class for transfer functions built from multiple geometries.
     * \tparam  T   Type of the base geometry class.
     */
    template<class T>
    class GenericGeometryTransferFunction : public AbstractTransferFunction, public sigslot::has_slots {
    public:
        /// Typedef for the geometry class this transfer function is built from.
        typedef T GeometryType;

        /**
         * Creates a new GenericGeometryTransferFunction.
         * \param   size            Size of the transfer function texture
         * \param   intensityDomain Intensity Domain where the transfer function is mapped to during classification
         */
        GenericGeometryTransferFunction(const cgt::vec3& size, const cgt::vec2& intensityDomain = cgt::vec2(0.f, 1.f));

        /**
         * Destructor, make sure to delete the OpenGL texture beforehand by calling deinit() with a valid OpenGL context!
         */
        virtual ~GenericGeometryTransferFunction();

        /**
         * Returns the intensity domain where this TF has it's non-transparent parts.
         * I.e. the minimum and the maximum intensity being opaque.
         * \return  cgt::vec2(0.f, 1.f)
         */
        virtual cgt::vec2 getVisibilityDomain() const;
        
        /**
         * Initializes the Shader, hence, this methods has to be called from a thread with a valid OpenGL context!
         */
        virtual void initShader();

        /**
         * Deletes the OpenGL texture and shader, hence, this methods has to be called from a thread with a valid OpenGL context!
         */
        virtual void deinit();

        /**
         * Gets the list of transfer function geometries.
         * \return  _geometries
         */
        const std::vector<T*>& getGeometries() const;

        /**
         * Adds the given TF geometry to this transfer function.
         * \note    GenericGeometryTransferFunction takes ownership \a geometry.
         * \param   geometry    TF geometry to add, GenericGeometryTransferFunction takes the ownership.
         */
        void addGeometry(T* geometry);

        /**
         * Removes the given TF geometry from this transfer function.
         * \note    After the call \a geometry will no longer be valid as GenericGeometryTransferFunction deletes the given T.
         * \param   geometry    TF geometry to remove, GenericGeometryTransferFunction will delete it
         */
        void removeGeometry(T* geometry);

        /**
         * Slot to be called by T's s_changed signal.
         */
        void onGeometryChanged();

        /// Signal to be emitted when the vector of T objects (_geometries) changed (The collection, not the actual geometry).
        sigslot::signal0 s_geometryCollectionChanged;

        /// Signal to be emitted when this TF object is about to be deleted.
        sigslot::signal0 s_aboutToBeDeleted;

    protected:
        /**
         * Creates the texture and uploads it to OpenGL.
         * Gets called by bind() with the local mutex already acquired.
         */
        virtual void createTexture();

        std::vector<T*> _geometries;        ///< The list of transfer function geometries.
        cgt::FramebufferObject* _fbo;       ///< The FBO used for render into texture.
        cgt::Shader* _shader;               ///< Shader for rendering the TF into a texture
    };

// ================================================================================================

    template<class T>
    campvis::GenericGeometryTransferFunction<T>::GenericGeometryTransferFunction(const cgt::vec3& size, const cgt::vec2& intensityDomain /*= cgt::vec2(0.f, 1.f)*/)
        : AbstractTransferFunction(size, intensityDomain)
        , _fbo(0)
        , _shader(0)
    {

    }

    template<class T>
    campvis::GenericGeometryTransferFunction<T>::~GenericGeometryTransferFunction() {
    }

    template<class T>
    cgt::vec2 campvis::GenericGeometryTransferFunction<T>::getVisibilityDomain() const {
        if (_geometries.empty())
            return cgt::vec2(-1.f, -1.f);
        else {
            cgt::vec2 minmax(1.f, 0.f);
            for (size_t i = 0; i < _geometries.size(); ++i) {
                minmax.x = std::min(minmax.x, _geometries[i]->getIntensityDomain().x);
                minmax.y = std::max(minmax.y, _geometries[i]->getIntensityDomain().y);
            }
            return minmax;
        }
    }

    template<class T>
    void campvis::GenericGeometryTransferFunction<T>::initShader() {
        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "core/glsl/passthrough.frag", "");
        if (_shader == nullptr) {
            LERROR("Could not create Shader for Rendering the TF into the lookup texture!");
        }
    }

    template<class T>
    void campvis::GenericGeometryTransferFunction<T>::deinit() {
        s_aboutToBeDeleted.triggerSignal(); // use trigger to force blocking signal handling in same thread

        for (typename std::vector<T*>::iterator it = _geometries.begin(); it != _geometries.end(); ++it) {
            (*it)->s_changed.disconnect(this);
            delete *it;
        }
        _geometries.clear();

        if (_fbo != 0) {
            delete _fbo;
            _fbo = 0;
        }

        ShdrMgr.dispose(_shader);
        _shader = 0;
        AbstractTransferFunction::deinit();
    }

    template<class T>
    const std::vector<T*>& campvis::GenericGeometryTransferFunction<T>::getGeometries() const {
        return  _geometries;
    }

    template<class T>
    void campvis::GenericGeometryTransferFunction<T>::addGeometry(T* geometry) {
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            _geometries.push_back(geometry);
        }
        geometry->s_changed.connect(this, &GenericGeometryTransferFunction<T>::onGeometryChanged);
        _dirtyTexture = true;
        s_geometryCollectionChanged.emitSignal();
        s_changed.emitSignal();
    }

    template<class T>
    void campvis::GenericGeometryTransferFunction<T>::removeGeometry(T* geometry) {
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            for (typename std::vector<T*>::iterator it = _geometries.begin(); it != _geometries.end(); ++it) {
                if (*it == geometry) {
                    _geometries.erase(it);
                    break;
                }
            }
        }
        geometry->s_changed.disconnect(this);
        delete geometry;
        _dirtyTexture = true;
        s_geometryCollectionChanged.emitSignal();
        s_changed.emitSignal();
    }

    template<class T>
    void campvis::GenericGeometryTransferFunction<T>::onGeometryChanged() {
        _dirtyTexture = true;
        s_changed.emitSignal();
    }

    template<class T>
    void campvis::GenericGeometryTransferFunction<T>::createTexture() {
        if (_shader == 0) {
            initShader();
        }

        // acqiure a new TextureUnit, so that we don't mess with other currently bound textures during texture upload...
        cgt::TextureUnit tfUnit;
        tfUnit.activate();

        // detach old texture from FBO and delete it
        if (_texture != 0 && _fbo != 0) {
            _fbo->activate();
            _fbo->detachAll();
            _fbo->deactivate();
        }
        delete _texture;

        // create FBO if needed
        if (_fbo == 0) 
            _fbo = new cgt::FramebufferObject();
        _fbo->activate();
        LGL_ERROR;

        // create texture
        GLenum dataType = GL_UNSIGNED_BYTE;
        GLenum type = GL_TEXTURE_1D;
        switch (getDimensionality()) {
            case 1:
                type = GL_TEXTURE_1D;
                break;
            case 2:
                type = GL_TEXTURE_2D;
                break;
            default:
                cgtAssert(false, "This TF dimensionality is currently not supported - you have to implement it yourself!");
                break;
        }

        _texture = new cgt::Texture(type, _size, GL_RGBA8, cgt::Texture::LINEAR);
        _texture->setWrapping(cgt::Texture::CLAMP_TO_EDGE);
        LGL_ERROR;

        // attach texture to FBO
        _fbo->attachTexture(_texture);
        if (! _fbo->isComplete()) {
            LERROR("Incomplete FBO.");
            _fbo->deactivate();
            return;
        }
        LGL_ERROR;

        // render TF geometries into texture
        glViewport(0, 0, _texture->getWidth(), _texture->getHeight());
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        _shader->activate();
        _shader->setUniform("_projectionMatrix", cgt::mat4::createOrtho(0, 1, 0, 1, -1, 1));
        LGL_ERROR;

        for (typename std::vector<T*>::const_iterator it = _geometries.begin(); it != _geometries.end(); ++it) {
            (*it)->render();
        }
        LGL_ERROR;

        // deactivate Shader and FBO
        _shader->deactivate();
        _fbo->detachTexture(GL_COLOR_ATTACHMENT0);
        _fbo->detachAll();
        _fbo->deactivate();
        LGL_ERROR;

        cgt::TextureUnit::setZeroUnit();
        _dirtyTexture = false;
    }
}

#endif // GENERICGEOMETRYTRANSFERFUNCTION_H__
