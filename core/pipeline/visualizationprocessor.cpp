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

#include "visualizationprocessor.h"

#include "tgt/textureunit.h"
#include "core/datastructures/imagedata.h"

namespace campvis {

    const std::string VisualizationProcessor::loggerCat_ = "CAMPVis.core.datastructures.VisualizationProcessor";

// ================================================================================================

    VisualizationProcessor::VisualizationProcessor(IVec2Property& renderTargetSize)
        : AbstractProcessor()
        , _fbo(0)
        , _renderTargetSize("renderTargetSize", "Canvas Size", renderTargetSize.getValue(), renderTargetSize.getMinValue(), renderTargetSize.getMaxValue(), renderTargetSize.getStepValue())
    {
        renderTargetSize.addSharedProperty(&_renderTargetSize);
        _renderTargetSize.s_changed.connect<VisualizationProcessor>(this, &VisualizationProcessor::onPropertyChanged);
    }

    VisualizationProcessor::~VisualizationProcessor() {

    }

    void VisualizationProcessor::init() {
        AbstractProcessor::init();
        _fbo = new tgt::FramebufferObject();
    }

    void VisualizationProcessor::deinit() {
        _fbo->detachAll();
        delete _fbo;
        AbstractProcessor::deinit();
    }

    void VisualizationProcessor::createAndAttachTexture(GLint internalFormat, GLenum attachment) {
        tgtAssert(_fbo->isActive(), "Trying to attach a texture while FBO is not bound!");

        // acqiure a new TextureUnit, so that we don't mess with other currently bound textures during texture upload...
        tgt::TextureUnit rtUnit;
        rtUnit.activate();

        // Set OpenGL pixel alignment to 1 to avoid problems with NPOT textures
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // create texture
        tgt::Texture* tex = 0;
        switch(internalFormat) {
            case GL_RGB:
                tex = new tgt::Texture(0, getRenderTargetSize(), GL_RGB, GL_RGB, GL_UNSIGNED_BYTE, tgt::Texture::LINEAR);
                break;
            case GL_RGB16F_ARB:
                tex = new tgt::Texture(0, getRenderTargetSize(), GL_RGB, GL_RGB16F_ARB, GL_FLOAT, tgt::Texture::LINEAR);
                break;
            case GL_RGBA:
                tex = new tgt::Texture(0, getRenderTargetSize(), GL_RGBA, GL_RGBA, GL_UNSIGNED_BYTE, tgt::Texture::LINEAR);
                break;
            case GL_RGBA8:
                tex = new tgt::Texture(0, getRenderTargetSize(), GL_RGBA, GL_RGBA8, GL_UNSIGNED_BYTE, tgt::Texture::LINEAR);
                break;
            case GL_RGBA16:
                tex = new tgt::Texture(0, getRenderTargetSize(), GL_RGBA, GL_RGBA16, GL_UNSIGNED_SHORT, tgt::Texture::LINEAR);
                break;
            case GL_RGBA16F:
                tex = new tgt::Texture(0, getRenderTargetSize(), GL_RGBA, GL_RGBA16F, GL_FLOAT, tgt::Texture::LINEAR);
                break;
            case GL_RGBA32F:
                tex = new tgt::Texture(0, getRenderTargetSize(), GL_RGBA, GL_RGBA32F, GL_FLOAT, tgt::Texture::LINEAR);
                break;

            case GL_DEPTH_COMPONENT16:
                tex = new tgt::Texture(0, getRenderTargetSize(), GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT16, GL_FLOAT, tgt::Texture::LINEAR);
                break;
            case GL_DEPTH_COMPONENT24:
                tex = new tgt::Texture(0, getRenderTargetSize(), GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT24, GL_FLOAT, tgt::Texture::LINEAR);
                break;
#ifdef GL_DEPTH_COMPONENT32F
            case GL_DEPTH_COMPONENT32F:
                break;
#endif

            default:
                tgtAssert(false, "Unknown internal format!");
        }
        tex->uploadTexture();
        tex->setWrapping(tgt::Texture::CLAMP_TO_EDGE);

        // attach texture to FBO
        _fbo->attachTexture(tex, attachment);
    }

    void VisualizationProcessor::createAndAttachTexture(GLint internalFormat) {
        GLenum attachment = 0;
        switch(internalFormat) {
            case GL_RGB:
            case GL_RGB16F_ARB:
            case GL_RGBA:
            case GL_RGBA8:
            case GL_RGBA16:
            case GL_RGBA16F:
            case GL_RGBA32F:
                if (_fbo->getNumColorAttachments() >= static_cast<size_t>(GpuCaps.getMaxColorAttachments())) {
                    tgtAssert(false, "Tried to attach more color textures to FBO than supported!");
                    LWARNING("Tried to attach more color textures to FBO than supported, aborted.");
                    return;
                }
                attachment = static_cast<GLenum>(GL_COLOR_ATTACHMENT0 + _fbo->getNumColorAttachments());
                break;

            case GL_DEPTH_COMPONENT16:
            case GL_DEPTH_COMPONENT24:
#ifdef GL_DEPTH_COMPONENT32F
            case GL_DEPTH_COMPONENT32F:
#endif
                tgtAssert(_fbo->getDepthAttachment() == 0, "Tried to attach more than one depth texture.");
                attachment = GL_DEPTH_ATTACHMENT;
                break;

            default:
                tgtAssert(false, "Unknown internal format!");
        }
        createAndAttachTexture(internalFormat, attachment);
    }

    tgt::ivec3 VisualizationProcessor::getRenderTargetSize() const {
        return tgt::ivec3(_renderTargetSize.getValue(), 1);
    }

    void VisualizationProcessor::createAndAttachColorTexture() {
        createAndAttachTexture(GL_RGBA8);
    }

    void VisualizationProcessor::createAndAttachDepthTexture() {
        createAndAttachTexture(GL_DEPTH_COMPONENT24);
    }

}
