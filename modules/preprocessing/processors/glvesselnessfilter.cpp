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

#include "glvesselnessfilter.h"

#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"

#include "core/classification/simpletransferfunction.h"
#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

#include "core/tools/quadrenderer.h"

namespace campvis {

    const std::string GlVesselnessFilter::loggerCat_ = "CAMPVis.modules.classification.GlVesselnessFilter";

    GlVesselnessFilter::GlVesselnessFilter(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputImage("InputImage", "Input (Gaussian Filtered) Image", "", DataNameProperty::READ)
        , p_outputImage("OutputImage", "Output Eigenvector Image", "GlVesselnessFilter.out", DataNameProperty::WRITE)
        , p_lod("Lod", "Min/Max LOD for Multi-Level Texture Lookup", tgt::vec2(1.f, 3.f), tgt::vec2(0.f), tgt::vec2(10.f), tgt::vec2(.5f), tgt::ivec2(1))
        , p_alpha("Alpha", "Alpha Value for Vesselness", .5f, .01f, 1.f, .1f, 2)
        , p_beta("Beta", "Beta Value for Vesselness", .5f, .01f, 1.f, .1f, 2)
        , p_gamma("Gamma", "Gamma Value for Vesselness", .001f, .0001f, .1f, .001f, 4)
        , p_theta("Theta", "Theta Value for Vesselness", .5f, .1f, 1.f, .1f, 2)
        , p_halfPrecisionOutput("HalfPrecisionOutput", "Use Half Precision (16 Bit) Output", true)
        , _shader(0)
    {
        addProperty(p_inputImage);
        addProperty(p_outputImage);

        addProperty(p_lod);
        addProperty(p_alpha);
        addProperty(p_beta);
        addProperty(p_gamma);
        addProperty(p_theta);

        addProperty(p_halfPrecisionOutput);
    }

    GlVesselnessFilter::~GlVesselnessFilter() {

    }

    void GlVesselnessFilter::init() {
        VisualizationProcessor::init();

        _shader = ShdrMgr.loadWithCustomGlslVersion("core/glsl/passthrough.vert", "", "modules/preprocessing/glsl/glvesselnessfilter.frag", "", "400");
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void GlVesselnessFilter::deinit() {
        ShdrMgr.dispose(_shader);
        VisualizationProcessor::deinit();
    }

    void GlVesselnessFilter::updateResult(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_inputImage.getValue());

        if (img != 0) {
            if (img.getImageData()->getNumChannels() == 1) {
                const tgt::svec3& size = img->getSize();

                tgt::TextureUnit inputUnit;
                inputUnit.activate();

                // create texture for result
                tgt::Texture* resultTexture = new tgt::Texture(0, tgt::ivec3(size), GL_RED, p_halfPrecisionOutput.getValue() ? GL_R16F : GL_R32F, GL_FLOAT, tgt::Texture::LINEAR);
                resultTexture->uploadTexture();

                //tgt::Texture* eigenvalueTexture = new tgt::Texture(0, tgt::ivec3(size), GL_RGB, GL_RGB16F, GL_FLOAT, tgt::Texture::LINEAR);
                //eigenvalueTexture->uploadTexture();

                // activate shader and bind textures
                _shader->activate();
                _shader->setUniform("_lod", p_lod.getValue());
                _shader->setUniform("_alpha", p_alpha.getValue());
                _shader->setUniform("_beta", p_beta.getValue());
                _shader->setUniform("_gamma", p_gamma.getValue());
                _shader->setUniform("_theta", p_theta.getValue());

                const tgt::Texture* tex = img->getTexture();
                img->bind(_shader, inputUnit);

                // enable mipmapping if not done yet
                if (tex->getFilter() != tgt::Texture::MIPMAP) {
                    const_cast<tgt::Texture*>(tex)->setFilter(tgt::Texture::MIPMAP);
                    LGL_ERROR;
                    glGenerateMipmap(GL_TEXTURE_3D);
                    LGL_ERROR;
                    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    LGL_ERROR;
                    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    LGL_ERROR;
                }

                // activate FBO and attach texture
                _fbo->activate();
                glViewport(0, 0, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));
//                 static const GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
//                 glDrawBuffers(2, buffers);

                // render quad to compute difference measure by shader
                for (int z = 0; z < static_cast<int>(size.z); ++z) {
                    float zTexCoord = static_cast<float>(z)/static_cast<float>(size.z) + .5f/static_cast<float>(size.z);
                    _shader->setUniform("_zTexCoord", zTexCoord);
                    _fbo->attachTexture(resultTexture, GL_COLOR_ATTACHMENT0, 0, z);
                    //_fbo->attachTexture(eigenvalueTexture, GL_COLOR_ATTACHMENT1, 0, z);
                    QuadRdr.renderQuad();
                }

//                glDrawBuffers(1, buffers);
                _fbo->detachAll();
                _fbo->deactivate();
                _shader->deactivate();

                // put resulting image into DataContainer
                ImageData* id = new ImageData(3, size, 1);
                ImageRepresentationGL::create(id, resultTexture);
                id->setMappingInformation(img->getParent()->getMappingInformation());
                data.addData(p_outputImage.getValue(), id);

                //ImageData* id2 = new ImageData(3, size, 3);
                //ImageRepresentationGL::create(id2, eigenvalueTexture);
                //id2->setMappingInformation(img->getParent()->getMappingInformation());
                //data.addData("eigenvalues", id2);

                tgt::TextureUnit::setZeroUnit();
                LGL_ERROR;
            }
            else {
                LERROR("Input image must be single-channel.");
            }
        }
        else {
            LDEBUG("No suitable input image found.");
        }
    }

}
