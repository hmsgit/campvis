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

#include "glimageresampler.h"

#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"
#include "tgt/texture.h"

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
        , _shader(0)
    {
        addProperty(&p_inputImage);
        addProperty(&p_outputImage);
        addProperty(&p_resampleScale);
    }

    GlImageResampler::~GlImageResampler() {

    }

    void GlImageResampler::init() {
        VisualizationProcessor::init();

        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/preprocessing/glsl/glimageresampler.frag", "", false);
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void GlImageResampler::deinit() {
        ShdrMgr.dispose(_shader);
        VisualizationProcessor::deinit();
    }

    void GlImageResampler::process(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_inputImage.getValue());

        if (img != 0) {
            tgt::vec3 originalSize(img->getSize());
            tgt::ivec3 resampledSize(originalSize * p_resampleScale.getValue());

            tgt::TextureUnit inputUnit;
            inputUnit.activate();

            // create texture for result
            tgt::Texture* resultTexture = new tgt::Texture(0, resampledSize, img->getTexture()->getFormat(), img->getTexture()->getInternalFormat(), img->getTexture()->getDataType(), tgt::Texture::LINEAR);
            resultTexture->uploadTexture();

            // activate shader and bind textures
            _shader->activate();
            img->bind(_shader, inputUnit);

            // activate FBO and attach texture
            _fbo->activate();
            glViewport(0, 0, static_cast<GLsizei>(resampledSize.x), static_cast<GLsizei>(resampledSize.y));

            // render quad to compute difference measure by shader
            for (int z = 0; z < resampledSize.z; ++z) {
                float zTexCoord = static_cast<float>(z)/static_cast<float>(resampledSize.z) + .5f/static_cast<float>(resampledSize.z);
                _shader->setUniform("_zTexCoord", zTexCoord);
                _fbo->attachTexture(resultTexture, GL_COLOR_ATTACHMENT0, 0, z);
                QuadRdr.renderQuad();
            }
            _fbo->detachAll();
            _fbo->deactivate();
            _shader->deactivate();

            // put resulting image into DataContainer
            ImageData* id = new ImageData(3, resampledSize, 1);
            ImageRepresentationGL::create(id, resultTexture);
            const ImageMappingInformation& imi = img->getParent()->getMappingInformation();
            id->setMappingInformation(ImageMappingInformation(img->getSize(), imi.getOffset(), imi.getVoxelSize() / p_resampleScale.getValue(), imi.getRealWorldMapping()));
            data.addData(p_outputImage.getValue(), id);

            tgt::TextureUnit::setZeroUnit();
            LGL_ERROR;
        }
        else {
            LERROR("No suitable input image found.");
        }

        validate(INVALID_RESULT);
    }

}