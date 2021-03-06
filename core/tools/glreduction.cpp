// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
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

#include "glreduction.h"

#include "cgt/logmanager.h"
#include "cgt/framebufferobject.h"
#include "cgt/shadermanager.h"
#include "cgt/texture.h"
#include "cgt/textureunit.h"

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
        , _shader1d(0)
        , _shader2d(0)
        , _shader3d(0)
        , _fbo(0)
    {
        _shader1d = ShdrMgr.load("core/glsl/passthrough.vert", "core/glsl/tools/glreduction.frag", generateGlslHeader(_reductionOperator) + "#define REDUCTION_1D\n");
        _shader2d = ShdrMgr.load("core/glsl/passthrough.vert", "core/glsl/tools/glreduction.frag", generateGlslHeader(_reductionOperator) + "#define REDUCTION_2D\n");
        _shader3d = ShdrMgr.load("core/glsl/passthrough.vert", "core/glsl/tools/glreduction.frag", generateGlslHeader(_reductionOperator) + "#define REDUCTION_3D\n");

        if (_shader1d == 0 || _shader2d == 0 || _shader3d == 0) {
            LERROR("Could not load Shader for OpenGL reduction. Reduction will not work!");
            return;
        }
    }

    GlReduction::~GlReduction() {
        ShdrMgr.dispose(_shader1d);
        ShdrMgr.dispose(_shader2d);
        ShdrMgr.dispose(_shader3d);
    }

    std::vector<float> GlReduction::reduce(const ImageData* image) {
        cgtAssert(image != 0, "Image must not be 0!");
        if (_shader1d == 0 || _shader2d == 0 || _shader3d == 0) {
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
    
    std::vector<float> GlReduction::reduce(const cgt::Texture* texture) {
        std::vector<float> toReturn;

        cgtAssert(texture != 0, "Image must not be 0!");
        if (_shader1d == 0 || _shader2d == 0 || _shader3d == 0) {
            LERROR("Could not load Shader for OpenGL reduction. Reduction will not work!");
            return toReturn;
        }
        if (texture == 0) {
            LERROR("Empty texture received - nothing to reduce!");
            return toReturn;
        }

        const cgt::ivec3& size = texture->getDimensions();
        cgt::ivec2 texSize = size.xy();

        // Set OpenGL pixel alignment to 1 to avoid problems with NPOT textures
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

        // get a free texture unit
        cgt::TextureUnit inputUnit;
        inputUnit.activate();

        // create temporary textures
        cgt::Texture* tempTextures[2];
        for (size_t i = 0; i < 2; ++i) {
            tempTextures[i] = new cgt::Texture(GL_TEXTURE_2D, cgt::ivec3(texSize, 1), GL_RGBA32F, cgt::Texture::NEAREST);
            tempTextures[i]->setWrapping(cgt::Texture::CLAMP_TO_EDGE);
        }

        const cgt::Texture* inputTex = texture;
        cgt::Texture* outputTex = tempTextures[1];

        // create and initialize FBO
        _fbo = new cgt::FramebufferObject();
        _fbo->activate();
        LGL_ERROR;

        // perform 3D reduction if needed
        if (texture->getDimensions().z > 1) {
            _shader3d->activate();
            _fbo->attachTexture(outputTex);

            inputTex->bind();
            _shader3d->setUniform("_texture", inputUnit.getUnitNumber());
            _shader3d->setUniform("_textureSize", size);

            glViewport(0, 0, texSize.x, texSize.y);
            QuadRdr.renderQuad();
            _shader3d->deactivate();

            inputTex = outputTex;
            outputTex = tempTextures[0];
            LGL_ERROR;
        }

        // perform 2D reduction if needed
        if (texture->getDimensions().y > 1) {
            _shader2d->activate();
            _fbo->attachTexture(outputTex);

            inputTex->bind();
            _shader2d->setUniform("_texture", inputUnit.getUnitNumber());
            _shader2d->setUniform("_textureSize", size.xy());

            glViewport(0, 0, texSize.x, 1);
            QuadRdr.renderQuad();
            _shader2d->deactivate();

            inputTex = outputTex;
            outputTex = (outputTex == tempTextures[1]) ? tempTextures[0] : tempTextures[1];
            LGL_ERROR;
        }

        // finally, perform 1D reduction if needed
        {
            _shader1d->activate();
            _fbo->attachTexture(outputTex);

            inputTex->bind();
            _shader1d->setUniform("_texture", inputUnit.getUnitNumber());
            _shader1d->setUniform("_textureSize", size.xy());

            glViewport(0, 0, 1, 1);
            QuadRdr.renderQuad();
            _shader1d->deactivate();
            LGL_ERROR;
        }

        // read back stuff
        GLenum readBackFormat = cgt::Texture::calcMatchingFormat(outputTex->getInternalFormat());
        size_t channels = outputTex->getNumChannels();
        toReturn.resize(channels);
        glReadBuffer(GL_COLOR_ATTACHMENT0);
        glReadPixels(0, 0, 1, 1, readBackFormat, GL_FLOAT, &toReturn.front());
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
                cgtAssert(false, "Should not reach this, wrong enum value?");
                return "";
                break;
        }
    }

}