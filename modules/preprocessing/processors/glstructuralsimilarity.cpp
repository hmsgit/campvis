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

#include "glstructuralsimilarity.h"

#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"
#include "cgt/texture.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"

#include "core/tools/quadrenderer.h"

namespace campvis {

    const std::string GlStructuralSimilarity::loggerCat_ = "CAMPVis.modules.classification.GlStructuralSimilarity";

    GlStructuralSimilarity::GlStructuralSimilarity(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputImage1("InputImage1", "Input Image 1", "", DataNameProperty::READ)
        , p_inputImage2("InputImage2", "Input Image 2", "", DataNameProperty::READ)
        , p_outputImage("OutputImage", "Output Image", "GlStructuralSimilarity.out", DataNameProperty::WRITE)
        , _shader2D(nullptr)
    {
        addProperty(p_inputImage1, INVALID_RESULT);
        addProperty(p_inputImage2, INVALID_RESULT);
        addProperty(p_outputImage);
    }

    GlStructuralSimilarity::~GlStructuralSimilarity() {

    }

    void GlStructuralSimilarity::init() {
        VisualizationProcessor::init();

        _shader2D = ShdrMgr.load("core/glsl/passthrough.vert", "modules/preprocessing/glsl/glstructuralsimilarity.frag", "");
    }

    void GlStructuralSimilarity::deinit() {
        ShdrMgr.dispose(_shader2D);
        VisualizationProcessor::deinit();
    }

    void GlStructuralSimilarity::updateResult(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img1(data, p_inputImage1.getValue());
        ImageRepresentationGL::ScopedRepresentation img2(data, p_inputImage2.getValue());

        if (img1 && img2) {
            cgt::vec3 originalSize(img1->getSize());

            cgt::TextureUnit img1Unit, img2Unit;
            img1Unit.activate();

            // create texture for result
            cgt::Texture* resultTexture = new cgt::Texture(GL_TEXTURE_2D, originalSize, img1->getTexture()->getInternalFormat(), cgt::Texture::LINEAR);
            LGL_ERROR;

            // activate shader and bind textures
            _shader2D->activate();
            img1->bind(_shader2D, img1Unit, "_image1", "_image1Params");
            img2->bind(_shader2D, img2Unit, "_image2", "_image2Params");
            LGL_ERROR;
            
            // activate FBO and attach texture
            _fbo->activate();
            _fbo->attachTexture(resultTexture);

            glViewport(0, 0, static_cast<GLsizei>(originalSize.x), static_cast<GLsizei>(originalSize.y));
            QuadRdr.renderQuad();
            LGL_ERROR;

            _fbo->detachAll();
            _fbo->deactivate();
            _shader2D->deactivate();

            // put resulting image into DataContainer
            ImageData* id = new ImageData(img1->getParent()->getDimensionality(), originalSize, img1->getParent()->getNumChannels());
            ImageRepresentationGL::create(id, resultTexture);
            id->setMappingInformation(img1->getParent()->getMappingInformation());
            data.addData(p_outputImage.getValue(), id);

            cgt::TextureUnit::setZeroUnit();
            LGL_ERROR;
        }
        else {
            LDEBUG("No suitable input image found.");
        }
    }

}

