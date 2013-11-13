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

#include "glreduction.h"

#include "tgt/logmanager.h"
#include "tgt/framebufferobject.h"
#include "tgt/shadermanager.h"
#include "tgt/texture.h"
#include "tgt/textureunit.h"

#include "core/datastructures/facegeometry.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/tools/quadrenderer.h"

#include <algorithm>

#define DIV_CEIL(x,y) ((x) > 0) ? (1 + ((x) - 1)/(y)) : ((x) / (y))

namespace campvis {

    const std::string GlReduction::loggerCat_ = "CAMPVis.modules.registration.GlReduction";

    GlReduction::GlReduction()
        : _shader(0)
        , _fbo(0)
        , _renderQuad(0)
    {
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "core/glsl/tools/glreduction.frag", "", false);
        if (_shader == 0) {
            LERROR("Could not load Shader for OpenGL reduction. Reduction will not work!");
            return;
        }

        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    GlReduction::~GlReduction() {

    }

    std::vector<float> GlReduction::reduce(const ImageData* image) {
        tgtAssert(image != 0, "Image must not be 0!");
        if (_shader == 0) {
            LERROR("Could not load Shader for OpenGL reduction. Reduction will not work!");
            return std::vector<float>();
        }
        if (image == 0) {
            LERROR("Empty image received - nothing to reduce!");
            return std::vector<float>();
        }

        const ImageRepresentationGL* repGl = image->getRepresentation<ImageRepresentationGL>();
        if (repGl == 0) {
            LERROR("Could not convert input image to OpenGL texture - no reduction possible!");
            return std::vector<float>();
        }

        return reduce(repGl->getTexture());
    }
    
    std::vector<float> GlReduction::reduce(const tgt::Texture* texture) {
        std::vector<float> toReturn;

        tgtAssert(texture != 0, "Image must not be 0!");
        if (_shader == 0) {
            LERROR("Could not load Shader for OpenGL reduction. Reduction will not work!");
            return toReturn;
        }
        if (texture == 0) {
            LERROR("Empty texture received - nothing to reduce!");
            return toReturn;
        }

        //tgtAssert(texture->getNumChannels() == 1, "Reduction of images with more than one channel currently not implemented! Somebody was too lazy (or stressed - deadline was close) to do that...");
        tgtAssert(texture->getDimensions().z == 1, "Reduction of 3D images not yet implemented! Somebody was too lazy (or stressed - deadline was close) to do that...");

        std::vector<float> readBackBuffer;
        const tgt::ivec3& size = texture->getDimensions();
        tgt::vec2 texCoordMultiplier(1.f);
        tgt::ivec2 currentSize = size.xy();
        reduceSizes(currentSize, texCoordMultiplier);

        // Set OpenGL pixel alignment to 1 to avoid problems with NPOT textures
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // get a free texture unit
        tgt::TextureUnit inputUnit;

        // create temporary textures
        for (size_t i = 0; i < 2; ++i) {
            //_tempTextures[i] = new tgt::Texture(0, tgt::ivec3(currentSize, 1), GL_RED, GL_R32F, GL_FLOAT, tgt::Texture::NEAREST);
            _tempTextures[i] = new tgt::Texture(0, tgt::ivec3(currentSize, 1), GL_RGBA, GL_RGBA32F, GL_FLOAT, tgt::Texture::NEAREST);
            _tempTextures[i]->uploadTexture();
            _tempTextures[i]->setWrapping(tgt::Texture::CLAMP);
        }
        _readTex = 0;
        _writeTex = 1;

        // create and initialize FBO
        _fbo = new tgt::FramebufferObject();
        _fbo->activate();
        LGL_ERROR;

        // perform first reduction step outside:
        _shader->activate();
        _fbo->attachTexture(_tempTextures[_readTex]);

        _shader->setIgnoreUniformLocationError(true);
        inputUnit.activate();
        texture->bind();
        _shader->setUniform("_texture", inputUnit.getUnitNumber());
        _shader->setUniform("_textureParams._size", tgt::vec2(size.xy()));
        _shader->setUniform("_textureParams._sizeRCP", tgt::vec2(1.f) / tgt::vec2(size.xy()));
        _shader->setUniform("_textureParams._numChannels", static_cast<int>(texture->getNumChannels()));
        _shader->setIgnoreUniformLocationError(false);

        glViewport(0, 0, currentSize.x, currentSize.y);
        _shader->setUniform("_texCoordsMultiplier", texCoordMultiplier);
        QuadRdr.renderQuad();
        LGL_ERROR;

        // perform reduction until 1x1 texture remains
        while (currentSize.x > 1 || currentSize.y > 1) {
            reduceSizes(currentSize, texCoordMultiplier);

            _fbo->attachTexture(_tempTextures[_writeTex]);
            _tempTextures[_readTex]->bind();

            glViewport(0, 0, currentSize.x, currentSize.y);
            _shader->setUniform("_texCoordsMultiplier", texCoordMultiplier);
            QuadRdr.renderQuad();
            LGL_ERROR;

            //_fbo->detachTexture(GL_COLOR_ATTACHMENT0);
            std::swap(_writeTex, _readTex);
        }

        _shader->deactivate();


        // read back stuff
        GLenum readBackFormat = _tempTextures[_readTex]->getFormat();
        size_t channels = _tempTextures[_readTex]->getNumChannels();
        toReturn.resize(currentSize.x * currentSize.y * channels);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(0, 0, currentSize.x, currentSize.y, readBackFormat, GL_FLOAT, &toReturn.front());
        LGL_ERROR;

        // clean up...
        _fbo->detachAll();
        _fbo->deactivate();
        delete _fbo;
        _fbo = 0;

        delete _tempTextures[0];
        delete _tempTextures[1];
        LGL_ERROR;

        return toReturn;
    }

    void GlReduction::reduceSizes(tgt::ivec2& currentSize, tgt::vec2& texCoordMultiplier) {
        if (currentSize.x > 1) {
            currentSize.x = DIV_CEIL(currentSize.x, 2);
            texCoordMultiplier.x /= 2.f;
        }
        if (currentSize.y > 1) {
            currentSize.y = DIV_CEIL(currentSize.y, 2);
            texCoordMultiplier.y /= 2.f;
        }

    }

}