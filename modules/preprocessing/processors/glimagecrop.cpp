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

#include "glimagecrop.h"

#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"
#include "cgt/texture.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"

#include "core/tools/quadrenderer.h"

namespace campvis {

    const std::string GlImageCrop::loggerCat_ = "CAMPVis.modules.classification.GlImageCrop";

    GlImageCrop::GlImageCrop(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputImage("InputImage", "Input Image", "", DataNameProperty::READ)
        , p_outputImage("OutputImage", "Output Image", "GlImageCrop.out", DataNameProperty::WRITE)
        , p_llf("Llf", "LLF of Cropped Region", cgt::ivec3(0), cgt::ivec3(0), cgt::ivec3(1))
        , p_urb("Urb", "URB of Cropped Region", cgt::ivec3(1), cgt::ivec3(0), cgt::ivec3(1))
    {
        addProperty(p_inputImage, INVALID_PROPERTIES | INVALID_RESULT);
        addProperty(p_outputImage);
        addProperty(p_llf);
        addProperty(p_urb);
    }

    GlImageCrop::~GlImageCrop() {

    }

    void GlImageCrop::init() {
        VisualizationProcessor::init();

        _shader2D = ShdrMgr.load("core/glsl/passthrough.vert", "modules/preprocessing/glsl/glimagecrop.frag", "#define GLIMAGECROP_2D\n");
        _shader2D->setAttributeLocation(0, "in_Position");
        _shader2D->setAttributeLocation(1, "in_TexCoord");

        _shader3D = ShdrMgr.load("core/glsl/passthrough.vert", "modules/preprocessing/glsl/glimagecrop.frag", "#define GLIMAGECROP_3D\n");
        _shader3D->setAttributeLocation(0, "in_Position");
        _shader3D->setAttributeLocation(1, "in_TexCoord");
    }

    void GlImageCrop::deinit() {
        ShdrMgr.dispose(_shader2D);
        ShdrMgr.dispose(_shader3D);
        VisualizationProcessor::deinit();
    }

    void GlImageCrop::updateResult(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_inputImage.getValue());

        if (img != 0) {
            cgt::ivec3 outputSize = cgt::abs(p_urb.getValue() - p_llf.getValue());
            bool isTexture2D = img->getParent()->getDimensionality() == 2;
            if (isTexture2D) {
                outputSize.z = 1;
            }

            cgt::TextureUnit inputUnit;
            inputUnit.activate();

            // create texture for result
            cgt::Texture* resultTexture = new cgt::Texture(isTexture2D ? GL_TEXTURE_2D : GL_TEXTURE_3D, outputSize, img->getTexture()->getInternalFormat(), cgt::Texture::LINEAR);

            // activate shader and bind textures
            auto shader = isTexture2D ? _shader2D : _shader3D;

            shader->activate();
            shader->setUniform("_offset", p_llf.getValue());
            shader->setUniform("_outputSize", outputSize);
            img->bind(shader, inputUnit);

            // activate FBO and attach texture
            _fbo->activate();
            glViewport(0, 0, static_cast<GLsizei>(outputSize.x), static_cast<GLsizei>(outputSize.y));

            // render quad to compute difference measure by shader
            for (int z = 0; z < outputSize.z; ++z) {
                if (!isTexture2D) {
                    shader->setUniform("_zTexel", z);
                }
                _fbo->attachTexture(resultTexture, GL_COLOR_ATTACHMENT0, 0, z);
                QuadRdr.renderQuad();
            }
            _fbo->detachAll();
            _fbo->deactivate();
            shader->deactivate();

            // put resulting image into DataContainer
            ImageData* id = new ImageData(isTexture2D ? 2 : 3, outputSize, img.getImageData()->getNumChannels());
            ImageRepresentationGL::create(id, resultTexture);
            const ImageMappingInformation& imi = img->getParent()->getMappingInformation();
            id->setMappingInformation(ImageMappingInformation(img->getSize(), imi.getOffset() + (cgt::vec3(p_llf.getValue()) * imi.getVoxelSize()), imi.getVoxelSize(), imi.getCustomTransformation()));
            data.addData(p_outputImage.getValue(), id);

            cgt::TextureUnit::setZeroUnit();
            LGL_ERROR;
        }
        else {
            LDEBUG("No suitable input image found.");
        }
    }

    void GlImageCrop::updateProperties(DataContainer& dataContainer) {
        ImageRepresentationGL::ScopedRepresentation img(dataContainer, p_inputImage.getValue());
        if (img != nullptr) {
            cgt::ivec3 size(img->getSize());

            if (p_llf.getMaxValue() != size-1)
                p_llf.setMaxValue(size-1);
            if (p_urb.getMaxValue() != size) {
                p_urb.setMaxValue(size);
                p_urb.setValue(size);
            }
        }
    }

}
