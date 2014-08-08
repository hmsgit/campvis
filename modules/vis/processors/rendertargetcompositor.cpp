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

#include "rendertargetcompositor.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/renderdata.h"


#include "core/classification/simpletransferfunction.h"

#include "core/tools/quadrenderer.h"

namespace campvis {

    static const GenericOption<RenderTargetCompositor::CompositingMode> compositingOptions[5] = {
        GenericOption<RenderTargetCompositor::CompositingMode>("first", "Only First", RenderTargetCompositor::CompositingModeFirst),
        GenericOption<RenderTargetCompositor::CompositingMode>("second", "Only Second", RenderTargetCompositor::CompositingModeSecond),
        GenericOption<RenderTargetCompositor::CompositingMode>("alpha", "Alpha Blending", RenderTargetCompositor::CompositingModeAlpha),
        GenericOption<RenderTargetCompositor::CompositingMode>("diff", "Difference", RenderTargetCompositor::CompositingModeDifference),
        GenericOption<RenderTargetCompositor::CompositingMode>("depth", "Depth Test", RenderTargetCompositor::CompositingModeDepth)
    };

    const std::string RenderTargetCompositor::loggerCat_ = "CAMPVis.modules.vis.RenderTargetCompositor";

    RenderTargetCompositor::RenderTargetCompositor(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_firstImageId("FirstImageId", "First Input Image", "", DataNameProperty::READ)
        , p_secondImageId("SecondImageId", "Second Input Image", "", DataNameProperty::READ)
        , p_targetImageId("TargetImageId", "Output Image", "", DataNameProperty::WRITE)
        , p_compositingMethod("CompositingMethod", "Compositing Method", compositingOptions, 5)
        , p_alphaValue("AlphaValue", "AlphaValue", .5f, 0.f, 1.f)
        , p_enableBackground("EnableBackground", "Enable Background", true)
        , _shader(0)
    {
        addProperty(p_firstImageId);
        addProperty(p_secondImageId);
        addProperty(p_targetImageId);
        addProperty(p_compositingMethod);
        addProperty(p_alphaValue);
        addProperty(p_enableBackground);

        addDecorator(new ProcessorDecoratorBackground());
        
        decoratePropertyCollection(this);
    }

    RenderTargetCompositor::~RenderTargetCompositor() {

    }

    void RenderTargetCompositor::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/vis/glsl/rendertargetcompositor.frag", "");
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void RenderTargetCompositor::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void RenderTargetCompositor::updateResult(DataContainer& data) {
        ScopedTypedData<RenderData> firstImage(data, p_firstImageId.getValue());
        ScopedTypedData<RenderData> secondImage(data, p_secondImageId.getValue());

        if ((p_compositingMethod.getOptionValue() == CompositingModeSecond || firstImage != nullptr) && (p_compositingMethod.getOptionValue() == CompositingModeFirst || secondImage != nullptr)) {
            FramebufferActivationGuard fag(this);
            createAndAttachColorTexture();
            createAndAttachDepthTexture();

            _shader->activate();
            tgt::TextureUnit firstColorUnit, firstDepthUnit, secondColorUnit, secondDepthUnit;

            if (p_compositingMethod.getOptionValue() != CompositingModeSecond)
                firstImage->bind(_shader, firstColorUnit, firstDepthUnit, "_firstColor", "_firstDepth", "_firstTexParams");
            if (p_compositingMethod.getOptionValue() != CompositingModeFirst)
                secondImage->bind(_shader, secondColorUnit, secondDepthUnit, "_secondColor", "_secondDepth", "_secondTexParams");

            _shader->setUniform("_compositingMethod", p_compositingMethod.getOptionValue());
            _shader->setUniform("_alpha", p_alphaValue.getValue());
            _shader->setUniform("_enableBackground", p_enableBackground.getValue());

            decorateRenderProlog(data, _shader);

            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_ALWAYS);
            glClearDepth(1.0f);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            QuadRdr.renderQuad();

            glDepthFunc(GL_LESS);
            glDisable(GL_DEPTH_TEST);

            _shader->deactivate();
            tgt::TextureUnit::setZeroUnit();
            LGL_ERROR;

            data.addData(p_targetImageId.getValue(), new RenderData(_fbo));
        }
        else {
            LDEBUG("No suitable input images found.");
        }
    }

}

