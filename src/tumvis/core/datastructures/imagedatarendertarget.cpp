#include "imagedatarendertarget.h"

#include "tgt/assert.h"
#include "tgt/gpucapabilities.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/tgt_gl.h"

namespace TUMVis {

    const std::string ImageDataRenderTarget::loggerCat_ = "TUMVis.core.datastructures.ImageDataRenderTarget";

    ImageDataRenderTarget::ImageDataRenderTarget(const tgt::svec3& size, GLint internalFormatColor /*= GL_RGBA8*/, GLint internalFormatDepth /*= GL_DEPTH_COMPONENT24*/)
        : ImageData(2, size)
        , _colorTexture(0)
        , _depthTexture(0)
        , _fbo(0)
    {
        tgtAssert(size.z == 1, "RenderTargets are only two-dimensional, expected size.z == 1.");

        initRenderTarget(internalFormatColor, internalFormatDepth);

        tgtAssert(_colorTexture != 0, "Color texture is 0, something went terribly wrong...");
        tgtAssert(_depthTexture != 0, "Depth texture is 0, something went terribly wrong...");
        tgtAssert(_fbo != 0, "Framebuffer object is 0, something went terribly wrong...");
    }

    ImageDataRenderTarget::~ImageDataRenderTarget() {
        delete _fbo;
        delete _colorTexture;
        delete _depthTexture;
    }

    void ImageDataRenderTarget::initRenderTarget(GLint internalFormatColor, GLint internalFormatDepth) {
        if (!GpuCaps.isNpotSupported() && !GpuCaps.areTextureRectanglesSupported()) {
            LWARNING("Neither non-power-of-two textures nor texture rectangles seem to be supported!");
        }

        switch(internalFormatColor) {
            case GL_RGB:
                _colorTexture = new tgt::Texture(0, _size, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, tgt::Texture::LINEAR);
                break;
            case GL_RGBA:
                _colorTexture = new tgt::Texture(0, _size, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, tgt::Texture::LINEAR);
                break;
            case GL_RGBA8:
                _colorTexture = new tgt::Texture(0, _size, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE, tgt::Texture::LINEAR);
                break;
            case GL_RGBA16:
                _colorTexture = new tgt::Texture(0, _size, GL_RGBA, GL_RGBA16, GL_UNSIGNED_SHORT, tgt::Texture::LINEAR);
                break;
            case GL_RGB16F_ARB:
                _colorTexture = new tgt::Texture(0, _size, GL_RGB, GL_RGB16F_ARB, GL_FLOAT, tgt::Texture::LINEAR);
                break;
            case GL_RGBA16F_ARB:
                _colorTexture = new tgt::Texture(0, _size, GL_RGBA, GL_RGBA16F_ARB, GL_FLOAT, tgt::Texture::LINEAR);
                break;
            case GL_RGBA32F_ARB:
                _colorTexture = new tgt::Texture(0, _size, GL_RGBA, GL_RGBA32F_ARB, GL_FLOAT, tgt::Texture::LINEAR);
                break;
            default:
                LERROR("Unknown internal format!");
        }
        _colorTexture->uploadTexture();
        _colorTexture->setWrapping(tgt::Texture::CLAMP_TO_EDGE);

        switch(internalFormatDepth) {
        case GL_DEPTH_COMPONENT16:
            _depthTexture = new tgt::Texture(0, _size, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT16, GL_FLOAT, tgt::Texture::LINEAR);
            break;
        case GL_DEPTH_COMPONENT24:
            _depthTexture = new tgt::Texture(0, _size, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT24, GL_FLOAT, tgt::Texture::LINEAR);
            break;
        case GL_DEPTH_COMPONENT32:
            _depthTexture = new tgt::Texture(0, _size, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32, GL_FLOAT, tgt::Texture::LINEAR);
            break;
#ifdef GL_DEPTH_COMPONENT32F
        case GL_DEPTH_COMPONENT32F:
            _depthTexture = new tgt::Texture(0, _size, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32F, GL_FLOAT, tgt::Texture::LINEAR);
            break;
#endif
        default:
            LERROR("Unknown internal depth format!");
        }
        _depthTexture->uploadTexture();
        _depthTexture->setWrapping(tgt::Texture::CLAMP_TO_EDGE);

        _fbo = new tgt::FramebufferObject();
        if (!_fbo) {
            LERROR("Failed to initialize framebuffer object!");
            return;
        }
        _fbo->activate();

        _fbo->attachTexture(_colorTexture);
        _fbo->isComplete();
        _fbo->attachTexture(_depthTexture, GL_DEPTH_ATTACHMENT_EXT);
        _fbo->isComplete();

        _fbo->deactivate();
    }

    void ImageDataRenderTarget::activate() {
        _fbo->activate();
        glViewport(0, 0, _size.x, _size.y);
    }

    void ImageDataRenderTarget::deactivate() {
        _fbo->deactivate();
    }

    void ImageDataRenderTarget::bindColorTexture() const {
        _colorTexture->bind();
    }

    void ImageDataRenderTarget::bindColorTexture(const tgt::TextureUnit& texUnit) const {
        texUnit.activate();
        _colorTexture->bind();
    }

    void ImageDataRenderTarget::bindDepthTexture() const {
        _depthTexture->bind();
    }

    void ImageDataRenderTarget::bindDepthTexture(const tgt::TextureUnit& texUnit) const {
        texUnit.activate();
        _depthTexture->bind();
    }

    void ImageDataRenderTarget::bind(tgt::Shader* shader, const tgt::TextureUnit* colorTexUnit, const tgt::TextureUnit* depthTexUnit, const std::string& colorTexUniform /*= "_colorTexture"*/, const std::string& depthTexUniform /*= "_depthTexture"*/) const {
        bool tmp = shader->getIgnoreUniformLocationError();
        shader->setIgnoreUniformLocationError(true);
        if (colorTexUnit != 0) {
            bindColorTexture(*colorTexUnit);
            shader->setUniform(colorTexUniform + "._texture", colorTexUnit->getUnitNumber());
            shader->setUniform(colorTexUniform + "._size", tgt::vec2(_size.xy()));
            shader->setUniform(colorTexUniform + "._sizeRCP", tgt::vec2(1.f) / tgt::vec2(_size.xy()));
        }
        if (depthTexUnit != 0) {
            bindDepthTexture(*depthTexUnit);
            shader->setUniform(depthTexUniform + "._texture", depthTexUnit->getUnitNumber());
            shader->setUniform(depthTexUniform + "._size", tgt::vec2(_size.xy()));
            shader->setUniform(depthTexUniform + "._sizeRCP", tgt::vec2(1.f) / tgt::vec2(_size.xy()));
        }
        shader->setIgnoreUniformLocationError(tmp);
    }

    ImageDataRenderTarget* ImageDataRenderTarget::clone() const {
        tgtAssert(false, "To be implemented!");
        return 0;
    }

    ImageDataRenderTarget* ImageDataRenderTarget::getSubImage(const tgt::svec3& llf, const tgt::svec3& urb) const {
        tgtAssert(false, "To be implemented!");
        return 0;
    }

}