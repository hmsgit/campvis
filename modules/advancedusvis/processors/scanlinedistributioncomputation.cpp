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

#include "scanlinedistributioncomputation.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorbackground.h"

#include "core/classification/simpletransferfunction.h"
#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

#include "core/tools/quadrenderer.h"

namespace campvis {
    const std::string ScanlineDistributionComputation::loggerCat_ = "CAMPVis.modules.vis.ScanlineDistributionComputation";


    ScanlineDistributionComputation::ScanlineDistributionComputation(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputImage("InputImage", "Input Image", "", DataNameProperty::READ)
        , p_outputImage("OutputImage", "Output Image", "predicatemask", DataNameProperty::WRITE)
        , p_scanningDirection("ScanningDirection", "US Scanning Direction", cgt::vec3(0.f, 1.f, 0.f), cgt::vec3(-1.f), cgt::vec3(1.f))
        , p_threshold("Threshold", "Threshold", .1f, .01f, 1.f)
        , p_stepSize("StepSize", "Step Size", 2.f, 1.f, 16.f, 1.f, 1)
        , _shader(0)
    {
        addProperty(p_inputImage, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_outputImage);

        addProperty(p_scanningDirection);
        addProperty(p_threshold);
        addProperty(p_stepSize);
    }

    ScanlineDistributionComputation::~ScanlineDistributionComputation() {

    }

    void ScanlineDistributionComputation::init() {
        VisualizationProcessor::init();

        _shader = ShdrMgr.loadWithCustomGlslVersion("core/glsl/passthrough.vert", "", "modules/advancedusvis/glsl/scanlinedistributioncomputation.frag", generateHeader(), "400");
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");

        _viewportSizeProperty->s_changed.disconnect(this);
        setPropertyInvalidationLevel(*_viewportSizeProperty, VALID);
    }

    void ScanlineDistributionComputation::deinit() {
        ShdrMgr.dispose(_shader);
        VisualizationProcessor::deinit();
    }

    void ScanlineDistributionComputation::updateResult(DataContainer& dataContainer) {
        ImageRepresentationGL::ScopedRepresentation img(dataContainer, p_inputImage.getValue());

        if (img != 0 && _shader != 0) {
            cgt::svec3 size = img->getSize() / static_cast<size_t>(2);
            cgt::ivec2 viewportSize = size.xy();

            cgt::TextureUnit inputUnit, gradientUnit;
            inputUnit.activate();

            // create texture for result
            cgt::Texture* distanceTexture = new cgt::Texture(0, cgt::ivec3(size), GL_RGB, GL_RGB16F, GL_FLOAT, cgt::Texture::LINEAR);
            distanceTexture->uploadTexture();
            distanceTexture->setWrapping(cgt::Texture::CLAMP);

            // activate shader and bind textures
            _shader->activate();
            _shader->setUniform("_scanningDirection", p_scanningDirection.getValue());
            _shader->setUniform("_intensityThreshold", p_threshold.getValue());
            _shader->setUniform("_stepSize", p_stepSize.getValue());

            img->bind(_shader, inputUnit);

            // activate FBO and attach texture
            _fbo->activate();
            glViewport(0, 0, static_cast<GLsizei>(viewportSize.x), static_cast<GLsizei>(viewportSize.y));

            // render quad to compute difference measure by shader
            for (int z = 0; z < static_cast<int>(size.z); ++z) {
                float zTexCoord = static_cast<float>(z)/static_cast<float>(size.z) + .5f/static_cast<float>(size.z);
                _shader->setUniform("_zTexCoord", zTexCoord);
                _fbo->attachTexture(distanceTexture, GL_COLOR_ATTACHMENT0, 0, z);
                QuadRdr.renderQuad();
            }
            _fbo->detachAll();
            _fbo->deactivate();
            _shader->deactivate();


            // put resulting image into DataContainer
            ImageData* id = new ImageData(3, size, 3);
            ImageRepresentationGL::create(id, distanceTexture);
            id->setMappingInformation(img->getParent()->getMappingInformation());
            dataContainer.addData(p_outputImage.getValue(), id);

            cgt::TextureUnit::setZeroUnit();
            LGL_ERROR;
        }
        else {
            LDEBUG("No suitable input image found.");
        }
    }

    void ScanlineDistributionComputation::updateProperties(DataContainer dataContainer) {
    }

    std::string ScanlineDistributionComputation::generateHeader() const {
        std::string toReturn = "";
        return toReturn;
    }

}
