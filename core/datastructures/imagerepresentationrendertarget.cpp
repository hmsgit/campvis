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

#include "imagerepresentationrendertarget.h"

#include "tgt/assert.h"
#include "tgt/gpucapabilities.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/tgt_gl.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"

namespace campvis {
    
    const std::string ImageRepresentationRenderTarget::loggerCat_ = "CAMPVis.core.datastructures.ImageRepresentationRenderTarget";

    ImageRepresentationRenderTarget* ImageRepresentationRenderTarget::create(ImageData* parent, GLint internalFormatColor /*= GL_RGBA8*/, GLint internalFormatDepth /*= GL_DEPTH_COMPONENT24*/) {
        ImageRepresentationRenderTarget* toReturn = new ImageRepresentationRenderTarget(parent, internalFormatColor, internalFormatDepth);
        toReturn->addToParent();
        return toReturn;
    }

    std::pair<ImageData*, ImageRepresentationRenderTarget*> ImageRepresentationRenderTarget::createWithImageData(const tgt::svec2& size, GLint internalFormatColor /*= GL_RGBA8*/, GLint internalFormatDepth /*= GL_DEPTH_COMPONENT24*/) {
        ImageData* id = new ImageData(2, tgt::svec3(size, 1), 4);
        ImageRepresentationRenderTarget* toReturn = new ImageRepresentationRenderTarget(id, internalFormatColor, internalFormatDepth);
        toReturn->addToParent();
        return std::make_pair(id, toReturn);
    }

    ImageRepresentationRenderTarget* ImageRepresentationRenderTarget::create(ImageData* parent, const tgt::FramebufferObject* fbo) {
        ImageRepresentationRenderTarget* toReturn = new ImageRepresentationRenderTarget(parent, fbo);
        toReturn->addToParent();
        return toReturn;
    }

    std::pair<ImageData*, ImageRepresentationRenderTarget*> ImageRepresentationRenderTarget::createWithImageData(const tgt::svec2& size, const tgt::FramebufferObject* fbo) {
        ImageData* id = new ImageData(2, tgt::svec3(size, 1), 4);
        ImageRepresentationRenderTarget* toReturn = new ImageRepresentationRenderTarget(id, fbo);
        toReturn->addToParent();
        return std::make_pair(id, toReturn);
    }

    ImageRepresentationRenderTarget::ImageRepresentationRenderTarget(ImageData* parent, GLint internalFormatColor /*= GL_RGBA8*/, GLint internalFormatDepth /*= GL_DEPTH_COMPONENT24*/)
        : GenericAbstractImageRepresentation<ImageRepresentationRenderTarget>(parent)
        , _colorTextures(0)
        , _depthTexture(0)
        , _fbo(0)
    {
        tgtAssert(parent->getSize().z == 1, "RenderTargets are only two-dimensional, expected parent image size.z == 1.");

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

    ImageRepresentationRenderTarget::ImageRepresentationRenderTarget(ImageData* parent, const ImageRepresentationGL* colorTexture, const ImageRepresentationGL* depthTexture /* = 0 */)
        : GenericAbstractImageRepresentation<ImageRepresentationRenderTarget>(parent)
        , _colorTextures(0)
        , _depthTexture(0)
        , _fbo(0)
    {
        tgtAssert(parent->getSize().z == 1, "RenderTargets are only two-dimensional, expected parent image size.z == 1.");
        tgtAssert(colorTexture != 0, "Color texture must not be 0.");

        tgtAssert(parent->getSize() == colorTexture->getSize(), "Texture size must match parent image size");
        tgtAssert(parent->getDimensionality() == colorTexture->getDimensionality(), "Texture dimensionality must match parent image dimensionality");
        tgtAssert(depthTexture == 0 || parent->getSize() == depthTexture->getSize(), "Texture size must match parent image size");
        tgtAssert(depthTexture == 0 || parent->getDimensionality() == depthTexture->getDimensionality(), "Texture dimensionality must match parent image dimensionality");

        if (!GpuCaps.isNpotSupported() && !GpuCaps.areTextureRectanglesSupported()) {
            LWARNING("Neither non-power-of-two textures nor texture rectangles seem to be supported!");
        }

        _fbo = new tgt::FramebufferObject();
        if (!_fbo) {
            LERROR("Failed to initialize framebuffer object!");
            return;
        }

        // TODO: finish implementation!
        tgtAssert(false, "Method not yet fully implemented!");

//         _fbo->activate();
//         tgt::Texture* cc = colorTexture->getTexture()
//         _fbo->attachTexture(cc, GL_COLOR_ATTACHMENT0);
    }

    ImageRepresentationRenderTarget::ImageRepresentationRenderTarget(ImageData* parent, const tgt::FramebufferObject* fbo)
        : GenericAbstractImageRepresentation<ImageRepresentationRenderTarget>(parent)
        , _colorTextures(0)
        , _depthTexture(0)
        , _fbo(0)
    {
        tgtAssert(parent->getSize().z == 1, "RenderTargets are only two-dimensional, expected parent image size.z == 1.");

        tgt::Texture *const *const attachments = fbo->getAttachments();
        for (size_t i = 0; i < TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS; ++i) {
            if (attachments[i] != 0)
                _colorTextures.push_back(attachments[i]);
        }
        if (attachments[TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS] != 0)
            _depthTexture = attachments[TGT_FRAMEBUFFEROBJECT_MAX_SUPPORTED_COLOR_ATTACHMENTS];
    }

    ImageRepresentationRenderTarget::~ImageRepresentationRenderTarget() {
        // TODO: double check whether deleting FBOs without detaching their attachments is not harmful
//         if (_fbo != 0) {
//             _fbo->activate();
//             _fbo->detachAll();
//             _fbo->deactivate();
//         }

        delete _fbo;
        for (std::vector<tgt::Texture*>::iterator it = _colorTextures.begin(); it != _colorTextures.end(); ++it)
            delete *it;
        delete _depthTexture;
    }
    
    void ImageRepresentationRenderTarget::activate() {
        _fbo->activate();
        glViewport(0, 0, static_cast<GLsizei>(getSize().x), static_cast<GLsizei>(getSize().y));
    }

    void ImageRepresentationRenderTarget::deactivate() {
        _fbo->deactivate();
    }

    void ImageRepresentationRenderTarget::bindColorTexture(tgt::Shader* shader, const tgt::TextureUnit& colorTexUnit, const std::string& colorTexUniform /*= "_colorTexture"*/, const std::string& texParamsUniform /*= "_texParams"*/, size_t index /*= 0*/) const {
        tgtAssert(index < _colorTextures.size(), "Color texture index out of bounds!");
        bool tmp = shader->getIgnoreUniformLocationError();
        shader->setIgnoreUniformLocationError(true);

        bindColorTexture(colorTexUnit, index);
        shader->setUniform(colorTexUniform, colorTexUnit.getUnitNumber());
        shader->setUniform(texParamsUniform + "._size", tgt::vec2(getSize().xy()));
        shader->setUniform(texParamsUniform + "._sizeRCP", tgt::vec2(1.f) / tgt::vec2(getSize().xy()));
        shader->setUniform(texParamsUniform + "._numChannels", static_cast<int>(_parent->getNumChannels()));

        shader->setIgnoreUniformLocationError(tmp);
    }

    void ImageRepresentationRenderTarget::bindDepthTexture(tgt::Shader* shader, const tgt::TextureUnit& depthTexUnit, const std::string& depthTexUniform /*= "_depthTexture"*/, const std::string& texParamsUniform /*= "_texParams"*/) const {
        bool tmp = shader->getIgnoreUniformLocationError();
        shader->setIgnoreUniformLocationError(true);

        bindDepthTexture(depthTexUnit);
        shader->setUniform(depthTexUniform, depthTexUnit.getUnitNumber());
        shader->setUniform(texParamsUniform + "._size", tgt::vec2(getSize().xy()));
        shader->setUniform(texParamsUniform + "._sizeRCP", tgt::vec2(1.f) / tgt::vec2(getSize().xy()));
        shader->setUniform(texParamsUniform + "._numChannels", static_cast<int>(_parent->getNumChannels()));

        shader->setIgnoreUniformLocationError(tmp);
    }

    void ImageRepresentationRenderTarget::bind(tgt::Shader* shader, const tgt::TextureUnit& colorTexUnit, const tgt::TextureUnit& depthTexUnit, const std::string& colorTexUniform /*= "_colorTexture"*/, const std::string& depthTexUniform /*= "_depthTexture"*/, const std::string& texParamsUniform /*= "_texParams"*/, size_t index /*= 0*/) const {
        tgtAssert(index < _colorTextures.size(), "Color texture index out of bounds!");
        bool tmp = shader->getIgnoreUniformLocationError();
        shader->setIgnoreUniformLocationError(true);

        bindColorTexture(colorTexUnit, index);
        bindDepthTexture(depthTexUnit);
        shader->setUniform(colorTexUniform, colorTexUnit.getUnitNumber());
        shader->setUniform(depthTexUniform, depthTexUnit.getUnitNumber());
        shader->setUniform(texParamsUniform + "._size", tgt::vec2(getSize().xy()));
        shader->setUniform(texParamsUniform + "._sizeRCP", tgt::vec2(1.f) / tgt::vec2(getSize().xy()));
        shader->setUniform(texParamsUniform + "._numChannels", static_cast<int>(_parent->getNumChannels()));

        shader->setIgnoreUniformLocationError(tmp);
    }

    ImageRepresentationRenderTarget* ImageRepresentationRenderTarget::clone(ImageData* newParent) const {
        tgtAssert(false, "To be implemented!");
        return 0;
    }

    ImageRepresentationRenderTarget* ImageRepresentationRenderTarget::getSubImage(ImageData* parent, const tgt::svec3& llf, const tgt::svec3& urb) const {
        tgtAssert(false, "To be implemented!");
        return 0;
    }

    const tgt::Texture* ImageRepresentationRenderTarget::getColorTexture(size_t index /*= 0*/) const {
        tgtAssert(index < _colorTextures.size(), "Color texture index out of bounds!");
       return _colorTextures[index];
    }

    const tgt::Texture* ImageRepresentationRenderTarget::getDepthTexture() const {
        return _depthTexture;
    }

    size_t ImageRepresentationRenderTarget::getNumColorTextures() const {
        return _colorTextures.size();
    }

    void ImageRepresentationRenderTarget::createAndAttachTexture(GLint internalFormat) {
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
                attachment = static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + _colorTextures.size());
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

        // Set OpenGL pixel alignment to 1 to avoid problems with NPOT textures
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // create texture
        tgt::Texture* tex = 0;
        switch(internalFormat) {
            case GL_RGB:
                tex = new tgt::Texture(0, getSize(), GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, tgt::Texture::LINEAR);
                _colorTextures.push_back(tex);
                break;
            case GL_RGB16F_ARB:
                tex = new tgt::Texture(0, getSize(), GL_RGB, GL_RGB16F_ARB, GL_FLOAT, tgt::Texture::LINEAR);
                _colorTextures.push_back(tex);
                break;
            case GL_RGBA:
                tex = new tgt::Texture(0, getSize(), GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, tgt::Texture::LINEAR);
                _colorTextures.push_back(tex);
                break;
            case GL_RGBA8:
                tex = new tgt::Texture(0, getSize(), GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE, tgt::Texture::LINEAR);
                _colorTextures.push_back(tex);
                break;
            case GL_RGBA16:
                tex = new tgt::Texture(0, getSize(), GL_RGBA, GL_RGBA16, GL_UNSIGNED_SHORT, tgt::Texture::LINEAR);
                _colorTextures.push_back(tex);
                break;
            case GL_RGBA16F:
                tex = new tgt::Texture(0, getSize(), GL_RGBA, GL_RGBA16F, GL_FLOAT, tgt::Texture::LINEAR);
                _colorTextures.push_back(tex);
                break;
            case GL_RGBA32F:
                tex = new tgt::Texture(0, getSize(), GL_RGBA, GL_RGBA32F, GL_FLOAT, tgt::Texture::LINEAR);
                _colorTextures.push_back(tex);
                break;

            case GL_DEPTH_COMPONENT16:
                tex = new tgt::Texture(0, getSize(), GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT16, GL_FLOAT, tgt::Texture::LINEAR);
                _depthTexture = tex;
                break;
            case GL_DEPTH_COMPONENT24:
                tex = new tgt::Texture(0, getSize(), GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT24, GL_FLOAT, tgt::Texture::LINEAR);
                _depthTexture = tex;
                break;
#ifdef GL_DEPTH_COMPONENT32F
            case GL_DEPTH_COMPONENT32F:
                tex = new tgt::Texture(0, getSize(), GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32F, GL_FLOAT, tgt::Texture::LINEAR);
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

    size_t ImageRepresentationRenderTarget::getLocalMemoryFootprint() const {
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

    size_t ImageRepresentationRenderTarget::getVideoMemoryFootprint() const {
        size_t sum = 0;
        for (std::vector<tgt::Texture*>::const_iterator it = _colorTextures.begin(); it != _colorTextures.end(); ++it)
            sum += (*it)->getSizeOnGPU();
       
        if (_depthTexture != 0)
            sum += _depthTexture->getSizeOnGPU();

        return sum;
    }

    ImageRepresentationRenderTarget* ImageRepresentationRenderTarget::tryConvertFrom(const AbstractImageRepresentation* source) {
        // no conversion availible for now
        return 0;
    }

    void ImageRepresentationRenderTarget::bindColorTexture(const tgt::TextureUnit& texUnit, size_t index /*= 0*/) const {
        tgtAssert(index < _colorTextures.size(), "Color texture index out of bounds!");
        texUnit.activate();
        _colorTextures[index]->bind();
    }

    void ImageRepresentationRenderTarget::bindDepthTexture(const tgt::TextureUnit& texUnit) const {
        texUnit.activate();
        _depthTexture->bind();
    }





}