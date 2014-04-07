// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "visualizationprocessor.h"

#include "tgt/textureunit.h"
#include "core/datastructures/imagedata.h"

namespace campvis {

    const std::string VisualizationProcessor::loggerCat_ = "CAMPVis.core.datastructures.VisualizationProcessor";

// ================================================================================================

    VisualizationProcessor::VisualizationProcessor(IVec2Property* viewportSizeProp)
        : AbstractProcessor()
        , p_lqMode("LqMode", "Low Quality Mode", false)
        , _fbo(0)
        , _viewportSizeProperty(viewportSizeProp)
    {
    }

    VisualizationProcessor::~VisualizationProcessor() {
    }

    void VisualizationProcessor::init() {
        AbstractProcessor::init();

        tgtAssert(_viewportSizeProperty != 0, "The pointer to the viewport size property must not be 0!");
        addProperty(p_lqMode);

        _fbo = new tgt::FramebufferObject();
        _viewportSizeProperty->s_changed.connect<VisualizationProcessor>(this, &VisualizationProcessor::onPropertyChanged);
        setPropertyInvalidationLevel(*_viewportSizeProperty, INVALID_RESULT);
    }

    void VisualizationProcessor::deinit() {
        _viewportSizeProperty->s_changed.disconnect(this);
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
                tex = new tgt::Texture(0, getRenderTargetSize(), GL_DEPTH_COMPONENT, GL_DEPTH_COMPONENT32F, GL_FLOAT, tgt::Texture::LINEAR);
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

    tgt::ivec2 VisualizationProcessor::getEffectiveViewportSize() const {
        return (p_lqMode.getValue() ? _viewportSizeProperty->getValue() / 2 : _viewportSizeProperty->getValue());
    }

    tgt::ivec3 VisualizationProcessor::getRenderTargetSize() const {
        return tgt::ivec3(getEffectiveViewportSize(), 1);
    }

    void VisualizationProcessor::createAndAttachColorTexture() {
        createAndAttachTexture(GL_RGBA8);
    }

    void VisualizationProcessor::createAndAttachDepthTexture() {
        createAndAttachTexture(GL_DEPTH_COMPONENT24);
    }

    void VisualizationProcessor::setViewportSizeProperty(IVec2Property* viewportSizeProp) {
        tgtAssert(viewportSizeProp != 0, "Pointer must not be 0.");

        if (_viewportSizeProperty != 0) {
            _viewportSizeProperty->s_changed.disconnect(this);
        }

        _viewportSizeProperty = viewportSizeProp;
        _viewportSizeProperty->s_changed.connect<VisualizationProcessor>(this, &VisualizationProcessor::onPropertyChanged);
        setPropertyInvalidationLevel(*viewportSizeProp, INVALID_RESULT);
    }

}
