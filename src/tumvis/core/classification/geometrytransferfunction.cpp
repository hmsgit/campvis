// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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

#include "geometrytransferfunction.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/texture.h"
#include "tgt/textureunit.h"

#include "core/classification/tfgeometry.h"

namespace TUMVis {

    const std::string GeometryTransferFunction::loggerCat_ = "TUMVis.core.classification.GeometryTransferFunction";

    GeometryTransferFunction::GeometryTransferFunction(size_t size, const tgt::vec2& intensityDomain /*= tgt::vec2(0.f, 1.f)*/) 
        : AbstractTransferFunction(tgt::svec3(size, 1, 1), intensityDomain)
    {
    }

    GeometryTransferFunction::~GeometryTransferFunction() {

    }

    size_t GeometryTransferFunction::getDimensionality() const {
        return 1;
    }

    void GeometryTransferFunction::createTexture() {
        delete _texture;

        GLenum dataType = GL_UNSIGNED_BYTE;
        _texture = new tgt::Texture(_size, GL_RGBA, dataType, tgt::Texture::LINEAR);
        _texture->setWrapping(tgt::Texture::CLAMP);

        GLubyte* ptr = _texture->getPixelData();
        memset(ptr, 0, _texture->getArraySize());

        for (std::vector<TFGeometry*>::const_iterator it = _geometries.begin(); it != _geometries.end(); ++it) {
            (*it)->rasterize(*_texture);
        }

        _texture->uploadTexture();
        _dirty = false;
    }

    const std::vector<TFGeometry*>& GeometryTransferFunction::getGeometries() const {
        return _geometries;
    }

    void GeometryTransferFunction::addGeometry(TFGeometry* geometry) {
        {
            tbb::mutex::scoped_lock lock(_localMutex);
            _geometries.push_back(geometry);
        }
        _dirty = true;
        s_changed();
    }


}