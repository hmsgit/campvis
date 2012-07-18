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

    void ImageDataGL::bind(tgt::Shader* shader, const tgt::TextureUnit& texUnit, const std::string& texUniform /*= "_texture"*/, const std::string& textureParametersUniform /*= "_textureParameters"*/) const {
        bind(texUnit);
        shader->setUniform(texUniform, texUnit.getUnitNumber());
        shader->setUniform(textureParametersUniform + "._size", tgt::vec2(_size.xy()));
        shader->setUniform(textureParametersUniform + "._sizeRCP", tgt::vec2(1.f) / tgt::vec2(_size.xy()));
    }

}