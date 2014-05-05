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

#include "depthdarkening.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"
#include "core/tools/glreduction.h"

#include "core/classification/simpletransferfunction.h"

#include "core/tools/quadrenderer.h"

namespace campvis {
    const std::string DepthDarkening::loggerCat_ = "CAMPVis.modules.vis.DepthDarkening";

    DepthDarkening::DepthDarkening(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputImage("InputImage", "Input Image", "", DataNameProperty::READ)
        , p_outputImage("OutputImage", "Output Image", "dd.output", DataNameProperty::WRITE)
        , p_sigma("Sigma", "Sigma of Gaussian Filter", 2.f, 0.f, 10.f, 0.1f)
        , p_lambda("Lambda", "Strength of Depth Darkening Effect", 10.f, 0.f, 50.f, 0.1f)
        , p_useColorCoding("UseColorCoding", "Cold/Warm Color Coding", false)
        , p_coldColor("ColdColor", "Cold Color (Far Objects)", tgt::vec3(0.f, 0.f, 1.f), tgt::vec3(0.f), tgt::vec3(1.f))
        , p_warmColor("WarmColor", "Warm Color (Near Objects)", tgt::vec3(1.f, 0.f, 0.f), tgt::vec3(0.f), tgt::vec3(1.f))
        , _shader(0)
        , _glReduction(0)
    {
        addProperty(p_inputImage);
        addProperty(p_outputImage);
        addProperty(p_sigma);
        addProperty(p_lambda);
        addProperty(p_useColorCoding, INVALID_RESULT | INVALID_SHADER);
        addProperty(p_coldColor);
        addProperty(p_warmColor);
    }

    DepthDarkening::~DepthDarkening() {

    }

    void DepthDarkening::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/vis/glsl/depthdarkening.frag", generateHeader());
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");

        _glReduction = new GlReduction(GlReduction::MIN_MAX_DEPTH_ONLY);
    }

    void DepthDarkening::deinit() {
        ShdrMgr.dispose(_shader);
        delete _glReduction;
        VisualizationProcessor::deinit();
    }

    void DepthDarkening::updateResult(DataContainer& data) {
        ScopedTypedData<RenderData> inputImage(data, p_inputImage.getValue());

        if (inputImage != 0 && inputImage->hasDepthTexture()) {
            const tgt::Texture* tex = inputImage->getDepthTexture()->getRepresentation<ImageRepresentationGL>()->getTexture();
            std::vector<float> tmp = _glReduction->reduce(tex);
            float minDepth = tmp[0];
            float maxDepth = tmp[1];

            FramebufferActivationGuard fag(this);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_ALWAYS);

            _shader->activate();
            tgt::TextureUnit colorUnit, depthUnit, pass2DepthUnit;
            inputImage->bind(_shader, colorUnit, depthUnit);
            inputImage->bindDepthTexture(_shader, pass2DepthUnit, "_depthPass2Texture", "_pass2TexParams");
            
            _shader->setUniform("_viewportSizeRCP", 1.f / tgt::vec2(getEffectiveViewportSize()));
            _shader->setUniform("_direction", tgt::vec2(1.f, 0.f));
            _shader->setUniform("_sigma", p_sigma.getValue());
            _shader->setUniform("_lambda", p_lambda.getValue());
            _shader->setUniform("_minDepth", minDepth);
            _shader->setUniform("_maxDepth", maxDepth);
            if (p_useColorCoding.getValue()) {
                _shader->setUniform("_coldColor", p_coldColor.getValue());
                _shader->setUniform("_warmColor", p_warmColor.getValue());
            }

            createAndAttachColorTexture();
            createAndAttachDepthTexture();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            QuadRdr.renderQuad();

            RenderData tempTarget(_fbo);
            _fbo->detachAll();
            createAndAttachColorTexture();
            createAndAttachDepthTexture();

            inputImage->bind(_shader, colorUnit, depthUnit);
            tempTarget.bindDepthTexture(_shader, pass2DepthUnit, "_depthPass2Texture", "_pass2TexParams");
            _shader->setUniform("_direction", tgt::vec2(0.f, 1.f));
            
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            QuadRdr.renderQuad();

            _shader->deactivate();
            tgt::TextureUnit::setZeroUnit();
            glDepthFunc(GL_LESS);
            glDisable(GL_DEPTH_TEST);
            LGL_ERROR;

            data.addData(p_outputImage.getValue(), new RenderData(_fbo));
        }
        else {
            LERROR("No suitable input image found.");
        }

        validate(INVALID_RESULT);
    }

    std::string DepthDarkening::generateHeader() const {
        if (p_useColorCoding.getValue())
            return "#define USE_COLORCODING\n";
        else
            return "";
    }

    void DepthDarkening::updateShader() {
        _shader->setHeaders(generateHeader());
        _shader->rebuild();
        validate(INVALID_SHADER);
    }

}

