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

#include "virtualmirrorcombine.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/renderdata.h"


#include "core/classification/simpletransferfunction.h"

#include "core/tools/quadrenderer.h"

namespace campvis {
    const std::string VirtualMirrorCombine::loggerCat_ = "CAMPVis.modules.vis.VirtualMirrorCombine";

    VirtualMirrorCombine::VirtualMirrorCombine(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_normalImageID("normalImageID", "Normal DVR Input Image", "", DataNameProperty::READ)
        , p_mirrorImageID("mirrorImageID", "Mirror DVR Input Image", "", DataNameProperty::READ)
        , p_mirrorRenderID("mirrorRenderID", "Rendered Mirror Input Image", "", DataNameProperty::READ)
        , p_targetImageID("targetImageID", "Output Image", "", DataNameProperty::WRITE)
        , _shader(0)
    {
        addProperty(p_normalImageID);
        addProperty(p_mirrorImageID);
        addProperty(p_mirrorRenderID);
        addProperty(p_targetImageID);

        addDecorator(new ProcessorDecoratorBackground());
        
        decoratePropertyCollection(this);
    }

    VirtualMirrorCombine::~VirtualMirrorCombine() {

    }

    void VirtualMirrorCombine::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/vis/glsl/virtualmirrorcombine.frag", "");
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void VirtualMirrorCombine::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void VirtualMirrorCombine::updateResult(DataContainer& data) {
        ScopedTypedData<RenderData> normalImage(data, p_normalImageID.getValue());
        ScopedTypedData<RenderData> mirrorImage(data, p_mirrorImageID.getValue());
        ScopedTypedData<RenderData> mirrorRendered(data, p_mirrorRenderID.getValue());

        if (normalImage != 0 && mirrorImage != 0 && mirrorRendered != 0) {
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_ALWAYS);

            FramebufferActivationGuard fag(this);
            createAndAttachColorTexture();
            createAndAttachDepthTexture();

            _shader->activate();
            decorateRenderProlog(data, _shader);

            tgt::TextureUnit normalColorUnit, normalDepthUnit, mirrorColorUnit, mirrorDepthUnit, mirrorRenderedDepthUnit;
            normalImage->bind(_shader, normalColorUnit, normalDepthUnit, "_normalColor", "_normalDepth", "_normalTexParams");
            mirrorImage->bind(_shader, mirrorColorUnit, mirrorDepthUnit, "_mirrorColor", "_mirrorDepth", "_mirrorTexParams");
            mirrorRendered->bindDepthTexture(_shader, mirrorRenderedDepthUnit, "_mirrorRenderedDepth", "_mirrorRenderedTexParams");

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            QuadRdr.renderQuad();

            _shader->deactivate();
            tgt::TextureUnit::setZeroUnit();
            glDepthFunc(GL_LESS);
            glDisable(GL_DEPTH_TEST);
            LGL_ERROR;

            data.addData(p_targetImageID.getValue(), new RenderData(_fbo));
        }
        else {
            LERROR("No suitable input images found.");
        }

        validate(INVALID_RESULT);
    }

}

