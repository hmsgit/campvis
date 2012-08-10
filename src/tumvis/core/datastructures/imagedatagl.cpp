// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "imagedatagl.h"
#include "tgt/assert.h"
#include "tgt/shadermanager.h"
#include "tgt/tgt_gl.h"

namespace TUMVis {

    const std::string ImageDataGL::loggerCat_ = "TUMVis.core.datastructures.ImageDataGL";

    ImageDataGL::ImageDataGL(size_t dimensionality, const tgt::svec3& size, const WeaklyTypedPointer& wtp) 
        : ImageData(dimensionality, size)
    {
        createTexture(wtp);
    }

    ImageDataGL::~ImageDataGL() {
        delete _texture;
    }

    ImageDataGL* ImageDataGL::clone() const {
        GLubyte* data = _texture->downloadTextureToBuffer();
        WeaklyTypedPointer wtp(WeaklyTypedPointer::baseType(_texture->getDataType()), WeaklyTypedPointer::numChannels(_texture->getFormat()), data);
        ImageDataGL* toReturn = new ImageDataGL(_dimensionality, _size, wtp);
        delete data;
        return toReturn;
    }

    ImageDataGL* ImageDataGL::getSubImage(const tgt::svec3& llf, const tgt::svec3& urb) const {
        // TODO: implement
        tgtAssert(false, "not yet implemented...");
        return 0;
    }

    void ImageDataGL::createTexture(const WeaklyTypedPointer& wtp) {
        tgtAssert(wtp._pointer != 0, "Pointer to image data must not be 0!");

        _texture = new tgt::Texture(reinterpret_cast<GLubyte*>(wtp._pointer), _size, wtp.getGlFormat(), wtp.getGlInternalFormat(), wtp.getGlDataType(), tgt::Texture::LINEAR);
        switch (_dimensionality) {
            case 1:
                _texture->setType(GL_TEXTURE_1D);
                break;
            case 2:
                _texture->setType(GL_TEXTURE_2D);
                break;
            case 3:
                _texture->setType(GL_TEXTURE_3D);
                break;
            default:
                tgtAssert(false, "Unsupported dimensionality of image.");
                break;
        }

        _texture->bind();
        _texture->uploadTexture();
        _texture->setWrapping(tgt::Texture::CLAMP);

        // TODO: map signed types

        // revoke ownership of local pixel data from the texture
        _texture->setPixelData(0);

        LGL_ERROR;
    }

    void ImageDataGL::bind() const {
        _texture->bind();
    }

    void ImageDataGL::bind(const tgt::TextureUnit& texUnit) const {
        texUnit.activate();
        _texture->bind();
    }

    void ImageDataGL::bind(tgt::Shader* shader, const tgt::TextureUnit& texUnit, const std::string& texUniform /*= "_texture"*/) const {
        bind(texUnit);
        bool tmp = shader->getIgnoreUniformLocationError();
        shader->setIgnoreUniformLocationError(true);

        switch (_dimensionality) {
        case 1:
            LERROR("Setting um 1D texture uniforms currently not implemented - you probably wanna do that yourself...");
            break;

        case 2:
            shader->setUniform(texUniform + "._texture", texUnit.getUnitNumber());
            shader->setUniform(texUniform + "._size", tgt::vec2(_size.xy()));
            shader->setUniform(texUniform + "._sizeRCP", tgt::vec2(1.f) / tgt::vec2(_size.xy()));
            break;

        case 3:
            shader->setUniform(texUniform + "._texture", texUnit.getUnitNumber());
            shader->setUniform(texUniform + "._size", tgt::vec3(_size));
            shader->setUniform(texUniform + "._sizeRCP", tgt::vec3(1.f) / tgt::vec3(_size));
            break;

        default:
            tgtAssert(false, "Should not reach this!");
            break;
        }
        shader->setIgnoreUniformLocationError(tmp);
    }

}