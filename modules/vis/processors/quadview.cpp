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

#include "quadview.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"

#include "core/datastructures/renderdata.h"


#include "core/classification/simpletransferfunction.h"

#include "core/tools/quadrenderer.h"

namespace campvis {
    const std::string QuadView::loggerCat_ = "CAMPVis.modules.vis.QuadView";

    QuadView::QuadView(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputImage1("InputImage1", "First Input Image", "", DataNameProperty::READ)
        , p_inputImage2("InputImage2", "Second Input Image", "", DataNameProperty::READ)
        , p_inputImage3("InputImage3", "Third Input Image", "", DataNameProperty::READ)
        , p_inputImage4("InputImage4", "Fourth Input Image", "", DataNameProperty::READ)
        , p_outputImage("OutputImage", "Output Image", "dd.output", DataNameProperty::WRITE)
        , _shader(0)
    {
        addProperty(p_inputImage1);
        addProperty(p_inputImage2);
        addProperty(p_inputImage3);
        addProperty(p_inputImage4);
        addProperty(p_outputImage);
    }

    QuadView::~QuadView() {

    }

    void QuadView::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/vis/glsl/quadview.frag", "");
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void QuadView::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void QuadView::updateResult(DataContainer& data) {
        ScopedTypedData<RenderData> inputImage1(data, p_inputImage1.getValue());
        ScopedTypedData<RenderData> inputImage2(data, p_inputImage2.getValue());
        ScopedTypedData<RenderData> inputImage3(data, p_inputImage3.getValue());
        ScopedTypedData<RenderData> inputImage4(data, p_inputImage4.getValue());

        if (inputImage1 == 0 && inputImage2 == 0 && inputImage3 == 0 && inputImage4 == 0) {
            return;
        }

        FramebufferActivationGuard fag(this);
        createAndAttachColorTexture();
        createAndAttachDepthTexture();

        tgt::TextureUnit colorUnit, depthUnit;
        _shader->activate();
        _shader->setUniform("_modelMatrix", tgt::mat4::createScale(tgt::vec3(.5f, .5f, .5f)));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (inputImage1 != 0) {
            inputImage1->bind(_shader, colorUnit, depthUnit);
            _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(-.5f, .5f, 0.f)));
            QuadRdr.renderQuad();
        }
        if (inputImage2 != 0) {
            inputImage2->bind(_shader, colorUnit, depthUnit);
            _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(.5f, .5f, 0.f)));
            QuadRdr.renderQuad();
        }
        if (inputImage3 != 0) {
            inputImage3->bind(_shader, colorUnit, depthUnit);
            _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(-.5f, -.5f, 0.f)));
            QuadRdr.renderQuad();
        }
        if (inputImage4 != 0) {
            inputImage4->bind(_shader, colorUnit, depthUnit);
            _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(.5f, -.5f, 0.f)));
            QuadRdr.renderQuad();
        }

        _shader->deactivate();
        tgt::TextureUnit::setZeroUnit();
        LGL_ERROR;

        data.addData(p_outputImage.getValue(), new RenderData(_fbo));
    }

}

