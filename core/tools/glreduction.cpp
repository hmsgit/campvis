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

    GlReduction::GlReduction(ReductionOperator reductionOperator)
        : _reductionOperator(reductionOperator)
        , _shader2d(0)
        , _shader3d(0)
        , _fbo(0)
    {
        _shader2d = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "core/glsl/tools/glreduction.frag", generateGlslHeader(_reductionOperator) + "#define REDUCTION_2D\n", false);
        _shader3d = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "core/glsl/tools/glreduction.frag", generateGlslHeader(_reductionOperator) + "#define REDUCTION_3D\n", false);
        if (_shader2d == 0 || _shader3d == 0) {
            LERROR("Could not load Shader for OpenGL reduction. Reduction will not work!");
            return;
        }

        _shader2d->setAttributeLocation(0, "in_Position");
        _shader2d->setAttributeLocation(1, "in_TexCoord");
        _shader3d->setAttributeLocation(0, "in_Position");
        _shader3d->setAttributeLocation(1, "in_TexCoord");
    }

    GlReduction::~GlReduction() {
        ShdrMgr.dispose(_shader2d);
        ShdrMgr.dispose(_shader3d);
    }

    std::vector<float> GlReduction::reduce(const ImageData* image) {
        tgtAssert(image != 0, "Image must not be 0!");
        if (_shader2d == 0 || _shader3d == 0) {
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
        if (_shader2d == 0 || _shader3d == 0) {
            LERROR("Could not load Shader for OpenGL reduction. Reduction will not work!");
            return toReturn;
        }
        if (texture == 0) {
            LERROR("Empty texture received - nothing to reduce!");
            return toReturn;
        }

        const tgt::ivec3& size = texture->getDimensions();
        tgt::vec2 texCoordShift = tgt::vec2(.5f) / tgt::vec2(size.xy());
        tgt::ivec2 currentSize = size.xy();
        reduceSizes(currentSize, texCoordShift);
        tgt::ivec2 startSize = currentSize;

        // Set OpenGL pixel alignment to 1 to avoid problems with NPOT textures
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // get a free texture unit
        tgt::TextureUnit inputUnit;

        // create temporary textures
        tgt::Texture* tempTextures[2];
        for (size_t i = 0; i < 2; ++i) {
            tempTextures[i] = new tgt::Texture(0, tgt::ivec3(currentSize, 1), GL_RGBA, GL_RGBA32F, GL_FLOAT, tgt::Texture::NEAREST);
            tempTextures[i]->uploadTexture();
            tempTextures[i]->setWrapping(tgt::Texture::CLAMP_TO_EDGE);
        }
        size_t readTex = 0;
        size_t writeTex = 1;

        // create and initialize FBO
        _fbo = new tgt::FramebufferObject();
        _fbo->activate();
        LGL_ERROR;

        // perform first reduction step outside:
        tgt::Shader* leShader = (texture->getDimensions().z == 1) ? _shader2d : _shader3d;
        leShader->activate();
        _fbo->attachTexture(tempTextures[readTex]);

        inputUnit.activate();
        texture->bind();
        leShader->setUniform("_texture", inputUnit.getUnitNumber());
        leShader->setUniform("_texCoordsShift", texCoordShift);
        if (leShader == _shader3d)
            leShader->setUniform("_textureDepth", texture->getDimensions().z);

        glViewport(startSize.x - currentSize.x, startSize.y - currentSize.y, currentSize.x, currentSize.y);
        QuadRdr.renderQuad();
        leShader->deactivate();
        LGL_ERROR;

        _shader2d->activate();
        _shader2d->setUniform("_texture", inputUnit.getUnitNumber());
        reduceSizes(currentSize, texCoordShift);
        glViewport(startSize.x - currentSize.x, startSize.y - currentSize.y, currentSize.x, currentSize.y);

        // perform reduction until 1x1 texture remains
        while (currentSize.x > 1 || currentSize.y > 1) {
            _fbo->attachTexture(tempTextures[writeTex]);
            tempTextures[readTex]->bind();

            _shader2d->setUniform("_texCoordsShift", texCoordShift);
            QuadRdr.renderQuad();
            LGL_ERROR;

            reduceSizes(currentSize, texCoordShift);
            std::swap(writeTex, readTex);
        }

        _shader2d->deactivate();


        // read back stuff
        GLenum readBackFormat = tempTextures[readTex]->getFormat();
        size_t channels = tempTextures[readTex]->getNumChannels();
        toReturn.resize(currentSize.x * currentSize.y * channels);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(startSize.x - currentSize.x, startSize.y - currentSize.y, currentSize.x, currentSize.y, readBackFormat, GL_FLOAT, &toReturn.front());
        LGL_ERROR;

        // clean up...
        _fbo->detachAll();
        _fbo->deactivate();
        delete _fbo;
        _fbo = 0;

        delete tempTextures[0];
        delete tempTextures[1];
        LGL_ERROR;

        return toReturn;
    }

    void GlReduction::reduceSizes(tgt::ivec2& currentSize, tgt::vec2& texCoordShift) {
        if (currentSize.x > 1) {
            currentSize.x = DIV_CEIL(currentSize.x, 2);
            if (currentSize.x == 1)
                texCoordShift.x *= -1.f;
        }
        if (currentSize.y > 1) {
            currentSize.y = DIV_CEIL(currentSize.y, 2);
            if (currentSize.y == 1)
                texCoordShift.y *= -1.f;
        }

    }

    std::string GlReduction::generateGlslHeader(ReductionOperator reductionOperator) {
        switch (reductionOperator) {
            case MIN:
                return 
                    "#define REDUCTION_OP_2(a, b) min(a, b)\n"
                    "#define REDUCTION_OP_4(a, b, c, d) min(a, min(b, min(c, d)))\n";
                break;
            case MAX:
                return 
                    "#define REDUCTION_OP_2(a, b) max(a, b)\n"
                    "#define REDUCTION_OP_4(a, b, c, d) max(a, max(b, max(c, d)))\n";
                break;
            case PLUS:
                return 
                    "#define REDUCTION_OP_2(a, b) a+b\n"
                    "#define REDUCTION_OP_4(a, b, c, d) a+b+c+d\n";
                break;
            case MULTIPLICATION:
                return 
                    "#define REDUCTION_OP_2(a, b) a*b\n"
                    "#define REDUCTION_OP_4(a, b, c, d) a*b*c*d\n";
                break;
            case MIN_MAX_DEPTH_ONLY:
                return 
                    "#define REDUCTION_OP_2(a, b) vec4(min(a.r, b.r), max(a.g, b.g), 0.0, 0.0)\n"
                    "#define REDUCTION_OP_4(a, b, c, d) vec4(min(a.r, min(b.r, min(c.r, d.r))), max(a.g, max(b.g, max(c.g, d.g))), 0.0, 0.0)\n";
                break;
            default:
                tgtAssert(false, "Should not reach this, wrong enum value?");
                return "";
                break;
        }
    }

}