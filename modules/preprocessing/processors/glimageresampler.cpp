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

#include "glimageresampler.h"

#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"
#include "cgt/texture.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"

#include "core/tools/quadrenderer.h"

namespace campvis {

    const std::string GlImageResampler::loggerCat_ = "CAMPVis.modules.classification.GlImageResampler";

    GlImageResampler::GlImageResampler(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputImage("InputImage", "Input Image", "", DataNameProperty::READ)
        , p_outputImage("OutputImage", "Output Image", "GlImageResampler.out", DataNameProperty::WRITE)
        , p_resampleScale("ResampleScale", "Resampling Scale", .5f, .01f, 10.f)
        , p_targetSize("TargetSize", "Size of Resampled Image", cgt::ivec3(128), cgt::ivec3(1), cgt::ivec3(1024))
        , _shader2D(0)
        , _shader3D(0)
    {
        addProperty(p_inputImage, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_outputImage);
        addProperty(p_resampleScale, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_targetSize);
    }

    GlImageResampler::~GlImageResampler() {

    }

    void GlImageResampler::init() {
        VisualizationProcessor::init();

        _shader2D = ShdrMgr.load("core/glsl/passthrough.vert", "modules/preprocessing/glsl/glimageresampler.frag", "#define GLRESAMPLER_2D\n");
        _shader2D->setAttributeLocation(0, "in_Position");
        _shader2D->setAttributeLocation(1, "in_TexCoord");

        _shader3D = ShdrMgr.load("core/glsl/passthrough.vert", "modules/preprocessing/glsl/glimageresampler.frag", "#define GLRESAMPLER_3D\n");
        _shader3D->setAttributeLocation(0, "in_Position");
        _shader3D->setAttributeLocation(1, "in_TexCoord");
    }

    void GlImageResampler::deinit() {
        ShdrMgr.dispose(_shader2D);
        ShdrMgr.dispose(_shader3D);
        VisualizationProcessor::deinit();
    }

    void GlImageResampler::updateResult(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_inputImage.getValue());

        if (img != 0) {
            cgt::vec3 originalSize(img->getSize());
            const cgt::ivec3& resampledSize = p_targetSize.getValue();
            bool isTexture2D = img->getParent()->getDimensionality() == 2;

            // 2D textures should not be scaled along the z axis
            if (isTexture2D) {
                resampledSize.z = 1;
            }

            cgt::TextureUnit inputUnit;
            inputUnit.activate();

            // create texture for result
            cgt::Texture* resultTexture = new cgt::Texture(isTexture2D ? GL_TEXTURE_2D : GL_TEXTURE_3D, resampledSize, img->getTexture()->getInternalFormat(), cgt::Texture::LINEAR);

            // Select the right shader for the 2D and 3D case
            cgt::Shader *shader = isTexture2D ? _shader2D : _shader3D;

            // activate shader and bind textures
            shader->activate();
            img->bind(shader, inputUnit);
            
            // activate FBO and attach texture
            _fbo->activate();
            glViewport(0, 0, static_cast<GLsizei>(resampledSize.x), static_cast<GLsizei>(resampledSize.y));

            // render quad to compute difference measure by shader
            for (int z = 0; z < resampledSize.z; ++z) {
                if (!isTexture2D) {    
                    float zTexCoord = static_cast<float>(z)/static_cast<float>(resampledSize.z) + .5f/static_cast<float>(resampledSize.z);
                    shader->setUniform("_zTexCoord", zTexCoord);
                }
                _fbo->attachTexture(resultTexture, GL_COLOR_ATTACHMENT0, 0, z);
                LGL_ERROR;
                QuadRdr.renderQuad();
            }
            _fbo->detachAll();
            _fbo->deactivate();
            shader->deactivate();

            // put resulting image into DataContainer
            ImageData* id = new ImageData(img->getParent()->getDimensionality(), resampledSize, img->getParent()->getNumChannels());
            ImageRepresentationGL::create(id, resultTexture);
            const ImageMappingInformation& imi = img->getParent()->getMappingInformation();
            id->setMappingInformation(ImageMappingInformation(img->getSize(), imi.getOffset(), imi.getVoxelSize() / p_resampleScale.getValue(), imi.getCustomTransformation()));
            data.addData(p_outputImage.getValue(), id);

            cgt::TextureUnit::setZeroUnit();
            LGL_ERROR;
        }
        else {
            LDEBUG("No suitable input image found.");
        }
    }

    void GlImageResampler::updateProperties(DataContainer& dataContainer) {
        ImageRepresentationGL::ScopedRepresentation img(dataContainer, p_inputImage.getValue());

        if (img != 0) {
            p_targetSize.setMaxValue(cgt::ivec3(img->getSize()) * int(p_resampleScale.getMaxValue()));
            p_targetSize.setValue(cgt::ivec3(cgt::vec3(img->getSize()) * p_resampleScale.getValue()));
        }
    }

}

