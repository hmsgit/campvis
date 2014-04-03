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

#include "glgaussianfilter.h"

#include "tgt/buffer.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"
#include "tgt/texture.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"

#include "core/tools/quadrenderer.h"
#include "core/tools/stringutils.h"

namespace campvis {

    const std::string GlGaussianFilter::loggerCat_ = "CAMPVis.modules.classification.GlGaussianFilter";

    #define MAX_SIGMA 20.f
    #define MAX_HALF_KERNEL_SIZE static_cast<int>(MAX_SIGMA * 2.5f) + 2

    GlGaussianFilter::GlGaussianFilter(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputImage("InputImage", "Input Image", "", DataNameProperty::READ)
        , p_outputImage("OutputImage", "Output Image", "GlGaussianFilter.out", DataNameProperty::WRITE)
        , p_sigma("Sigma", "Sigma (relates to kernel size)", 2.5f, 1.f, MAX_SIGMA, .1f, 1)
        , _shader(0)
        , _kernelBuffer(0)
        , _kernelBufferTexture(0)
    {
        addProperty(&p_inputImage);
        addProperty(&p_outputImage);
        addProperty(&p_sigma);
    }

    GlGaussianFilter::~GlGaussianFilter() {

    }

    void GlGaussianFilter::init() {
        VisualizationProcessor::init();

        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/preprocessing/glsl/glgaussianfilter.frag", "");
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");

        // create kernel buffer
        tgt::TextureUnit inputUnit;
        inputUnit.activate();

        _kernelBuffer = new tgt::BufferObject(tgt::BufferObject::TEXTURE_BUFFER, tgt::BufferObject::USAGE_STATIC_DRAW);
        glGenTextures(1, &_kernelBufferTexture);
        LGL_ERROR;
    }

    void GlGaussianFilter::deinit() {
        ShdrMgr.dispose(_shader);
        delete _kernelBuffer;
        glDeleteTextures(1, &_kernelBufferTexture);

        VisualizationProcessor::deinit();
    }

    void GlGaussianFilter::updateResult(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_inputImage.getValue());

        if (img != 0) {            
            tgt::ivec3 size = img->getSize();
            int halfKernelSize = static_cast<int>(2.5 * p_sigma.getValue());
            tgtAssert(halfKernelSize < MAX_HALF_KERNEL_SIZE, "halfKernelSize too big -> kernel uniform buffer will be out of bounds!")

            tgt::TextureUnit inputUnit, kernelUnit;
            inputUnit.activate();

            // create texture for result
            tgt::Texture* resultTextures[2];
            for (size_t i = 0; i < 2; ++i) {
                resultTextures[i] = new tgt::Texture(0, size, img->getTexture()->getFormat(), img->getTexture()->getInternalFormat(), img->getTexture()->getDataType(), tgt::Texture::LINEAR);
                resultTextures[i]->uploadTexture();
            }

            // create and upload kernel buffer
            GLfloat kernel[MAX_HALF_KERNEL_SIZE];
            for (int i = 0; i <= halfKernelSize; ++i) {
                kernel[i] = exp(- static_cast<GLfloat>(i*i) / (2.f * p_sigma.getValue() * p_sigma.getValue()));
            }
            _kernelBuffer->data(kernel, (halfKernelSize + 1) * sizeof(GLfloat), tgt::BufferObject::FLOAT, 1);

            // activate shader
            _shader->activate();
            _shader->setUniform("_halfKernelSize", halfKernelSize);

            // bind kernel buffer texture
            kernelUnit.activate();
            glBindTexture(GL_TEXTURE_BUFFER, _kernelBufferTexture);
            glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, _kernelBuffer->getId());
            _shader->setUniform("_kernel", kernelUnit.getUnitNumber());
            LGL_ERROR;

            // activate FBO and attach texture
            _fbo->activate();
            glViewport(0, 0, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));

            // start 3 passes of convolution: in X, Y and Z direction:
            {
                // X pass
                _shader->setUniform("_direction", tgt::ivec3(1, 0, 0));
                img->bind(_shader, inputUnit);

                // render quad to compute difference measure by shader
                for (int z = 0; z < size.z; ++z) {
                    float zTexCoord = static_cast<float>(z)/static_cast<float>(size.z) + .5f/static_cast<float>(size.z);
                    _shader->setUniform("_zTexCoord", zTexCoord);
                    _fbo->attachTexture(resultTextures[0], GL_COLOR_ATTACHMENT0, 0, z);
                    QuadRdr.renderQuad();
                }
            }
            {
                // Y pass
                _shader->setUniform("_direction", tgt::ivec3(0, 1, 0));
                inputUnit.activate();
                resultTextures[0]->bind();

                // render quad to compute difference measure by shader
                for (int z = 0; z < size.z; ++z) {
                    float zTexCoord = static_cast<float>(z)/static_cast<float>(size.z) + .5f/static_cast<float>(size.z);
                    _shader->setUniform("_zTexCoord", zTexCoord);
                    _fbo->attachTexture(resultTextures[1], GL_COLOR_ATTACHMENT0, 0, z);
                    QuadRdr.renderQuad();
                }
            }
            {
                // Z pass
                _shader->setUniform("_direction", tgt::ivec3(0, 0, 1));
                inputUnit.activate();
                resultTextures[1]->bind();

                // render quad to compute difference measure by shader
                for (int z = 0; z < size.z; ++z) {
                    float zTexCoord = static_cast<float>(z)/static_cast<float>(size.z) + .5f/static_cast<float>(size.z);
                    _shader->setUniform("_zTexCoord", zTexCoord);
                    _fbo->attachTexture(resultTextures[0], GL_COLOR_ATTACHMENT0, 0, z);
                    QuadRdr.renderQuad();
                }
            }

            _fbo->detachAll();
            _fbo->deactivate();
            _shader->deactivate();

            // put resulting image into DataContainer
            ImageData* id = new ImageData(3, size, img->getParent()->getNumChannels());
            ImageRepresentationGL::create(id, resultTextures[0]);
            id->setMappingInformation(img->getParent()->getMappingInformation());
            data.addData(p_outputImage.getValue(), id);

            delete resultTextures[1];

            tgt::TextureUnit::setZeroUnit();
            LGL_ERROR;
        }
        else {
            LERROR("No suitable input image found.");
        }

        validate(INVALID_RESULT);
    }
    
}
