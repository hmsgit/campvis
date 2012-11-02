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

#ifndef GENERICGEOMETRYTRANSFERFUNCTION_H__
#define GENERICGEOMETRYTRANSFERFUNCTION_H__

#include "core/classification/abstracttransferfunction.h"

#include "tgt/assert.h"
#include "tgt/framebufferobject.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/texture.h"
#include "tgt/textureunit.h"

#include <vector>

namespace campvis {

    /**
     * Generic base class for transfer functions built from multiple geometries.
     * \tparam  T   Type of the base geometry class.
     */
    template<class T>
    class GenericGeometryTransferFunction : public AbstractTransferFunction, public sigslot::has_slots<> {
    public:
        /// Typedef for the geometry class this transfer function is built from.
        typedef T GeometryType;

        /**
         * Creates a new GenericGeometryTransferFunction.
         * \param   size            Size of the transfer function texture
         * \param   intensityDomain Intensity Domain where the transfer function is mapped to during classification
         */
        GenericGeometryTransferFunction(const tgt::vec3& size, const tgt::vec2& intensityDomain = tgt::vec2(0.f, 1.f));

        /**
         * Destructor, make sure to delete the OpenGL texture beforehand by calling deinit() with a valid OpenGL context!
         */
        virtual ~GenericGeometryTransferFunction();

        /**
         * Deletes the OpenGL texture, hence, this methods has to be called from a thread with a valid OpenGL context!
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
        sigslot::signal0<> s_geometryCollectionChanged;

    protected:
        /**
         * Creates the texture and uploads it to OpenGL.
         * Gets called by bind() with the local mutex already acquired.
         */
        virtual void createTexture();

        std::vector<T*> _geometries;        ///< The list of transfer function geometries.
        tgt::FramebufferObject* _fbo;       ///< The FBO used for render into texture.
    };

// ================================================================================================

    template<class T>
    campvis::GenericGeometryTransferFunction<T>::GenericGeometryTransferFunction(const tgt::vec3& size, const tgt::vec2& intensityDomain /*= tgt::vec2(0.f, 1.f)*/)
        : AbstractTransferFunction(size, intensityDomain)
        , _fbo(0)
    {

    }

    template<class T>
    campvis::GenericGeometryTransferFunction<T>::~GenericGeometryTransferFunction() {
        for (std::vector<T*>::iterator it = _geometries.begin(); it != _geometries.end(); ++it) {
            (*it)->s_changed.disconnect(this);
            delete *it;
        }
    }

    template<class T>
    void campvis::GenericGeometryTransferFunction<T>::deinit() {
        if (_fbo != 0) {
            _fbo->activate();
            _fbo->detachAll();
            _fbo->deactivate();
            LGL_ERROR;
            delete _fbo;
        }

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
        s_geometryCollectionChanged();
        s_changed();
    }

    template<class T>
    void campvis::GenericGeometryTransferFunction<T>::removeGeometry(T* geometry) {
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            for (std::vector<T*>::iterator it = _geometries.begin(); it != _geometries.end(); ++it) {
                if (*it == geometry) {
                    _geometries.erase(it);
                    break;
                }
            }
        }
        geometry->s_changed.disconnect(this);
        delete geometry;
        _dirtyTexture = true;
        s_geometryCollectionChanged();
        s_changed();
    }

    template<class T>
    void campvis::GenericGeometryTransferFunction<T>::onGeometryChanged() {
        _dirtyTexture = true;
        s_changed();
    }

    template<class T>
    void campvis::GenericGeometryTransferFunction<T>::createTexture() {
        // acqiure a new TextureUnit, so that we don't mess with other currently bound textures during texture upload...
        tgt::TextureUnit tfUnit;
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
            _fbo = new tgt::FramebufferObject();
        _fbo->activate();
        LGL_ERROR;

        // create texture
        GLenum dataType = GL_UNSIGNED_BYTE;
        _texture = new tgt::Texture(_size, GL_RGBA, dataType, tgt::Texture::LINEAR);
        _texture->setWrapping(tgt::Texture::CLAMP);
        _texture->uploadTexture();
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
        glPushAttrib(GL_ALL_ATTRIB_BITS);
        glClearColor(0, 0, 0, 0);
        glClear(GL_COLOR_BUFFER_BIT);

        glViewport(0, 0, _texture->getWidth(), _texture->getHeight());
        glMatrixMode(GL_PROJECTION);
        glPushMatrix();
        glLoadIdentity();
        glOrtho(0, 1, 0, 1, -1, 1);

        glMatrixMode(GL_MODELVIEW);
        glPushMatrix();
        glLoadIdentity();


        for (std::vector<T*>::const_iterator it = _geometries.begin(); it != _geometries.end(); ++it) {
            (*it)->render();
        }

        glMatrixMode(GL_PROJECTION);
        glPopMatrix();
        glMatrixMode(GL_MODELVIEW);
        glPopMatrix();
        glPopAttrib();
        LGL_ERROR;

        // deactivate FBO
        _fbo->deactivate();
        LGL_ERROR;

        tgt::TextureUnit::setZeroUnit();
        _dirtyTexture = false;
    }
}

#endif // GENERICGEOMETRYTRANSFERFUNCTION_H__
