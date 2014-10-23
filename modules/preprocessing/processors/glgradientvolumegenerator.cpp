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

#include "glgradientvolumegenerator.h"

#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorgradient.h"

#include "core/classification/simpletransferfunction.h"
#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

#include "core/tools/quadrenderer.h"

namespace campvis {

    const std::string GlGradientVolumeGenerator::loggerCat_ = "CAMPVis.modules.classification.GlGradientVolumeGenerator";

    GlGradientVolumeGenerator::GlGradientVolumeGenerator(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputImage("InputImage", "Input Image", "", DataNameProperty::READ)
        , p_outputImage("OutputImage", "Output Image", "GlGradientVolumeGenerator.out", DataNameProperty::WRITE)
        , _shader(0)
    {
        addDecorator(new ProcessorDecoratorGradient());

        addProperty(p_inputImage);
        addProperty(p_outputImage);
        decoratePropertyCollection(this);
    }

    GlGradientVolumeGenerator::~GlGradientVolumeGenerator() {

    }

    void GlGradientVolumeGenerator::init() {
        VisualizationProcessor::init();

        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/preprocessing/glsl/glgradientvolumegenerator.frag", generateHeader());
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void GlGradientVolumeGenerator::deinit() {
        ShdrMgr.dispose(_shader);
        VisualizationProcessor::deinit();
    }

    void GlGradientVolumeGenerator::updateResult(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_inputImage.getValue());

        if (img != 0) {
            const cgt::svec3& size = img->getSize();

            cgt::TextureUnit inputUnit;
            inputUnit.activate();

            // create texture for result
            cgt::Texture* resultTexture = new cgt::Texture(0, cgt::ivec3(size), GL_RGB, GL_RGB16F, GL_FLOAT, cgt::Texture::LINEAR);
            resultTexture->uploadTexture();

            // activate shader and bind textures
            _shader->activate();
            img->bind(_shader, inputUnit);

            // activate FBO and attach texture
            _fbo->activate();
            glViewport(0, 0, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));

            // render quad to compute difference measure by shader
            for (int z = 0; z < static_cast<int>(size.z); ++z) {
                float zTexCoord = static_cast<float>(z)/static_cast<float>(size.z) + .5f/static_cast<float>(size.z);
                _shader->setUniform("_zTexCoord", zTexCoord);
                _fbo->attachTexture(resultTexture, GL_COLOR_ATTACHMENT0, 0, z);
                QuadRdr.renderQuad();
            }
            _fbo->detachAll();
            _fbo->deactivate();
            _shader->deactivate();

            // put resulting image into DataContainer
            ImageData* id = new ImageData(3, size, 3);
            ImageRepresentationGL::create(id, resultTexture);
            id->setMappingInformation(img->getParent()->getMappingInformation());
            data.addData(p_outputImage.getValue(), id);

            cgt::TextureUnit::setZeroUnit();
            LGL_ERROR;
        }
        else {
            LDEBUG("No suitable input image found.");
        }
    }

    std::string GlGradientVolumeGenerator::generateHeader() const {
        return getDecoratedHeader();
    }

    void GlGradientVolumeGenerator::updateShader() {
        _shader->setHeaders(generateHeader());
        _shader->rebuild();
    }

}
