// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

#include "glgaussianfilter.h"

#include "cgt/buffer.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"
#include "cgt/texture.h"

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
        , _shader2D(0)
        , _shader3D(0)
        , _kernelBuffer(0)
        , _kernelBufferTexture(0)
    {
        addProperty(p_inputImage);
        addProperty(p_outputImage);
        addProperty(p_sigma);
    }

    GlGaussianFilter::~GlGaussianFilter() {

    }

    void GlGaussianFilter::init() {
        VisualizationProcessor::init();

        _shader2D = ShdrMgr.load("core/glsl/passthrough.vert", "modules/preprocessing/glsl/glgaussianfilter.frag", "#define GAUSSIAN_2D\n");
        _shader2D->setAttributeLocation(0, "in_Position");
        _shader2D->setAttributeLocation(1, "in_TexCoord");

        _shader3D = ShdrMgr.load("core/glsl/passthrough.vert", "modules/preprocessing/glsl/glgaussianfilter.frag", "#define GAUSSIAN_3D\n");
        _shader3D->setAttributeLocation(0, "in_Position");
        _shader3D->setAttributeLocation(1, "in_TexCoord");

        // create kernel buffer
        cgt::TextureUnit inputUnit;
        inputUnit.activate();

        _kernelBuffer = new cgt::BufferObject(cgt::BufferObject::TEXTURE_BUFFER, cgt::BufferObject::USAGE_STATIC_DRAW);
        glGenTextures(1, &_kernelBufferTexture);
        LGL_ERROR;
    }

    void GlGaussianFilter::deinit() {
        ShdrMgr.dispose(_shader2D);
        ShdrMgr.dispose(_shader3D);
        delete _kernelBuffer;
        glDeleteTextures(1, &_kernelBufferTexture);

        VisualizationProcessor::deinit();
    }

    void GlGaussianFilter::updateResult(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_inputImage.getValue());

        if (img != 0) {
            if (img->getParent()->getDimensionality() > 1) {
                cgt::ivec3 size = img->getSize();
                int halfKernelSize = static_cast<int>(2.5 * p_sigma.getValue());
                cgtAssert(halfKernelSize < MAX_HALF_KERNEL_SIZE, "halfKernelSize too big -> kernel uniform buffer will be out of bounds!")

                cgt::TextureUnit inputUnit, kernelUnit;
                inputUnit.activate();

                // create texture for result
                cgt::Texture* resultTextures[2];
                for (size_t i = 0; i < 2; ++i) {
                    resultTextures[i] = new cgt::Texture(0, size, img->getTexture()->getFormat(), img->getTexture()->getInternalFormat(), img->getTexture()->getDataType(), cgt::Texture::LINEAR);
                    resultTextures[i]->uploadTexture();
                }

                // create and upload kernel buffer
                GLfloat kernel[MAX_HALF_KERNEL_SIZE];
                for (int i = 0; i <= halfKernelSize; ++i) {
                    kernel[i] = exp(- static_cast<GLfloat>(i*i) / (2.f * p_sigma.getValue() * p_sigma.getValue()));
                }
                _kernelBuffer->data(kernel, (halfKernelSize + 1) * sizeof(GLfloat), cgt::BufferObject::FLOAT, 1);

                // we need to distinguish 2D and 3D case
                cgt::Shader* leShader = (size.z == 1) ? _shader2D : _shader3D;

                // activate shader
                leShader->activate();
                leShader->setUniform("_halfKernelSize", halfKernelSize);

                // bind kernel buffer texture
                kernelUnit.activate();
                glBindTexture(GL_TEXTURE_BUFFER, _kernelBufferTexture);
                glTexBuffer(GL_TEXTURE_BUFFER, GL_R32F, _kernelBuffer->getId());
                leShader->setUniform("_kernel", kernelUnit.getUnitNumber());
                LGL_ERROR;

                // activate FBO and attach texture
                _fbo->activate();
                glViewport(0, 0, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));

                // start 3 passes of convolution: in X, Y and Z direction:
                {
                    // X pass
                    leShader->setUniform("_direction", cgt::ivec3(1, 0, 0));
                    img->bind(leShader, inputUnit);

                    // render quad to compute difference measure by shader
                    for (int z = 0; z < size.z; ++z) {
                        float zTexCoord = static_cast<float>(z)/static_cast<float>(size.z) + .5f/static_cast<float>(size.z);
                        if (size.z > 1)
                            leShader->setUniform("_zTexCoord", zTexCoord);
                        _fbo->attachTexture(resultTextures[0], GL_COLOR_ATTACHMENT0, 0, z);
                        LGL_ERROR;
                        QuadRdr.renderQuad();
                    }
                }
                {
                    // Y pass
                    leShader->setUniform("_direction", cgt::ivec3(0, 1, 0));
                    inputUnit.activate();
                    resultTextures[0]->bind();

                    // render quad to compute difference measure by shader
                    for (int z = 0; z < size.z; ++z) {
                        float zTexCoord = static_cast<float>(z)/static_cast<float>(size.z) + .5f/static_cast<float>(size.z);
                        if (size.z > 1)
                            leShader->setUniform("_zTexCoord", zTexCoord);
                        _fbo->attachTexture(resultTextures[1], GL_COLOR_ATTACHMENT0, 0, z);
                        LGL_ERROR;
                        QuadRdr.renderQuad();
                    }
                }
                // we need the third pass only in the 3D case
                if (size.z > 1) {
                    // Z pass
                    leShader->setUniform("_direction", cgt::ivec3(0, 0, 1));
                    inputUnit.activate();
                    resultTextures[1]->bind();

                    // render quad to compute difference measure by shader
                    for (int z = 0; z < size.z; ++z) {
                        float zTexCoord = static_cast<float>(z)/static_cast<float>(size.z) + .5f/static_cast<float>(size.z);
                        leShader->setUniform("_zTexCoord", zTexCoord);
                        _fbo->attachTexture(resultTextures[0], GL_COLOR_ATTACHMENT0, 0, z);
                        LGL_ERROR;
                        QuadRdr.renderQuad();
                    }
                }
                else {
                    // in the 2D case we just swap the result textures, so that we write the correct image out in the lines below.
                    std::swap(resultTextures[0], resultTextures[1]);
                }

                _fbo->detachAll();
                _fbo->deactivate();
                leShader->deactivate();

                // put resulting image into DataContainer
                ImageData* id = new ImageData(3, size, img->getParent()->getNumChannels());
                ImageRepresentationGL::create(id, resultTextures[0]);
                id->setMappingInformation(img->getParent()->getMappingInformation());
                data.addData(p_outputImage.getValue(), id);

                delete resultTextures[1];

                cgt::TextureUnit::setZeroUnit();
                LGL_ERROR;
            }
            else {
                LERROR("Supports only 2D and 3D Gaussian Blur.");
            }
        }
        else {
            LDEBUG("No suitable input image found.");
        }
    }
    
}
