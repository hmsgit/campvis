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
        , _colorTextures(0)
        , _depthTexture(0)
        , _fbo(0)
    {
        tgtAssert(size.z == 1, "RenderTargets are only two-dimensional, expected size.z == 1.");

        if (!GpuCaps.isNpotSupported() && !GpuCaps.areTextureRectanglesSupported()) {
            LWARNING("Neither non-power-of-two textures nor texture rectangles seem to be supported!");
        }

        _fbo = new tgt::FramebufferObject();
        if (!_fbo) {
            LERROR("Failed to initialize framebuffer object!");
            return;
        }

        createAndAttachTexture(internalFormatColor);
        createAndAttachTexture(internalFormatDepth);

        tgtAssert(_colorTextures.front() != 0, "Color texture is 0, something went terribly wrong...");
        tgtAssert(_depthTexture != 0, "Depth texture is 0, something went terribly wrong...");
        tgtAssert(_fbo != 0, "Framebuffer object is 0, something went terribly wrong...");
    }

    ImageDataRenderTarget::~ImageDataRenderTarget() {
        if (_fbo != 0) {
            _fbo->activate();
            _fbo->detachAll();
            _fbo->deactivate();
        }

        delete _fbo;
        for (std::vector<tgt::Texture*>::iterator it = _colorTextures.begin(); it != _colorTextures.end(); ++it)
            delete *it;
        delete _depthTexture;
    }
    
    void ImageDataRenderTarget::activate() {
        _fbo->activate();
        glViewport(0, 0, _size.x, _size.y);
    }

    void ImageDataRenderTarget::deactivate() {
        _fbo->deactivate();
    }

    void ImageDataRenderTarget::bindColorTexture(size_t index /*= 0*/) const {
        tgtAssert(index < _colorTextures.size(), "Color texture index out of bounds!");
        _colorTextures[index]->bind();
    }

    void ImageDataRenderTarget::bindColorTexture(const tgt::TextureUnit& texUnit, size_t index /*= 0*/) const {
        tgtAssert(index < _colorTextures.size(), "Color texture index out of bounds!");
        texUnit.activate();
        _colorTextures[index]->bind();
    }

    void ImageDataRenderTarget::bindDepthTexture() const {
        _depthTexture->bind();
    }

    void ImageDataRenderTarget::bindDepthTexture(const tgt::TextureUnit& texUnit) const {
        texUnit.activate();
        _depthTexture->bind();
    }

    void ImageDataRenderTarget::bind(tgt::Shader* shader, const tgt::TextureUnit* colorTexUnit, const tgt::TextureUnit* depthTexUnit, const std::string& colorTexUniform /*= "_colorTextures"*/, const std::string& depthTexUniform /*= "_depthTexture"*/, size_t index /*= 0*/) const {
        tgtAssert(index < _colorTextures.size(), "Color texture index out of bounds!");
        bool tmp = shader->getIgnoreUniformLocationError();
        shader->setIgnoreUniformLocationError(true);
        if (colorTexUnit != 0) {
            bindColorTexture(*colorTexUnit, index);
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

    const tgt::Texture* ImageDataRenderTarget::getColorTexture(size_t index /*= 0*/) const {
        tgtAssert(index < _colorTextures.size(), "Color texture index out of bounds!");
       return _colorTextures[index];
    }

    const tgt::Texture* ImageDataRenderTarget::getDepthTexture() const {
        return _depthTexture;
    }

    size_t ImageDataRenderTarget::getNumColorTextures() const {
        return _colorTextures.size();
    }

    void ImageDataRenderTarget::createAndAttachTexture(GLint internalFormat) {
        // do sanity tests:
        GLenum attachment = 0;
        switch(internalFormat) {
            case GL_RGB:
            case GL_RGB16F_ARB:
            case GL_RGBA:
            case GL_RGBA8:
            case GL_RGBA16:
            case GL_RGBA16F:
            case GL_RGBA32F:
                if (_colorTextures.size() >= static_cast<size_t>(GpuCaps.getMaxColorAttachments())) {
                    tgtAssert(false, "Tried to attach more color textures to FBO than supported!");
                    LWARNING("Tried to attach more color textures to FBO than supported, aborted.");
                    return;
                }
                attachment = GL_COLOR_ATTACHMENT0 + _colorTextures.size();
                break;

            case GL_DEPTH_COMPONENT16:
            case GL_DEPTH_COMPONENT24:
#ifdef GL_DEPTH_COMPONENT32F
            case GL_DEPTH_COMPONENT32F:
#endif
                tgtAssert(_depthTexture == 0, "Tried to attach more than one depth texture.");
                attachment = GL_DEPTH_ATTACHMENT;
                break;

            default:
                tgtAssert(false, "Unknown internal format!");
        }

        // acqiure a new TextureUnit, so that we don't mess with other currently bound textures during texture upload...
        tgt::TextureUnit rtUnit;
        rtUnit.activate();

        // create texture
        tgt::Texture* tex = 0;
        switch(internalFormat) {
            case GL_RGB:
                tex = new tgt::Texture(0, _size, GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, tgt::Texture::LINEAR);
                _colorTextures.push_back(tex);
                break;
            case GL_RGB16F_ARB:
                tex = new tgt::Texture(0, _size, GL_RGB, GL_RGB16F_ARB, GL_FLOAT, tgt::Texture::LINEAR);
                _colorTextures.push_back(tex);
                break;
            case GL_RGBA:
                tex = new tgt::Texture(0, _size, GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, tgt::Texture::LINEAR);
                _colorTextures.push_back(tex);
                break;
            case GL_RGBA8:
                tex = new tgt::Texture(0, _size, GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE, tgt::Texture::LINEAR);
                _colorTextures.push_back(tex);
                break;
            case GL_RGBA16:
                tex = new tgt::Texture(0, _size, GL_RGBA, GL_RGBA16, GL_UNSIGNED_SHORT, tgt::Texture::LINEAR);
                _colorTextures.push_back(tex);
                break;
            case GL_RGBA16F:
                tex = new tgt::Texture(0, _size, GL_RGBA, GL_RGBA16F, GL_FLOAT, tgt::Texture::LINEAR);
                _colorTextures.push_back(tex);
                break;
            case GL_RGBA32F:
                tex = new tgt::Texture(0, _size, GL_RGBA, GL_RGBA32F, GL_FLOAT, tgt::Texture::LINEAR);
                _colorTextures.push_back(tex);
                break;

            case GL_DEPTH_COMPONENT16:
                tex = new tgt::Texture(0, _size, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT16, GL_FLOAT, tgt::Texture::LINEAR);
                _depthTexture = tex;
                break;
            case GL_DEPTH_COMPONENT24:
                tex = new tgt::Texture(0, _size, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT24, GL_FLOAT, tgt::Texture::LINEAR);
                _depthTexture = tex;
                break;
#ifdef GL_DEPTH_COMPONENT32F
            case GL_DEPTH_COMPONENT32F:
                tex = new tgt::Texture(0, _size, GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32F, GL_FLOAT, tgt::Texture::LINEAR);
                break;
#endif

            default:
                tgtAssert(false, "Unknown internal format!");
        }
        tex->uploadTexture();
        tex->setWrapping(tgt::Texture::CLAMP_TO_EDGE);

        // attach texture to FBO
        _fbo->activate();
        _fbo->attachTexture(tex, attachment);
        _fbo->isComplete();
        _fbo->deactivate();
    }

    size_t ImageDataRenderTarget::getLocalMemoryFootprint() const {
        size_t sum = 0;
        sum += sizeof(tgt::FramebufferObject);

        sum += sizeof(tgt::Texture) * _colorTextures.size();
        for (std::vector<tgt::Texture*>::const_iterator it = _colorTextures.begin(); it != _colorTextures.end(); ++it) {
            if ((*it)->getPixelData() != 0)
                sum += (*it)->getBpp() * (*it)->getArraySize();
        }

        if (_depthTexture != 0 && _depthTexture->getPixelData() != 0) {
            sum += sizeof(tgt::Texture);
            sum += _depthTexture->getBpp() * _depthTexture->getArraySize();
        }

        return sizeof(*this) + sum;
    }

    size_t ImageDataRenderTarget::getVideoMemoryFootprint() const {
        size_t sum = 0;
        for (std::vector<tgt::Texture*>::const_iterator it = _colorTextures.begin(); it != _colorTextures.end(); ++it)
            sum += (*it)->getSizeOnGPU();
       
        if (_depthTexture != 0)
            sum += _depthTexture->getSizeOnGPU();

        return sum;
    }

}