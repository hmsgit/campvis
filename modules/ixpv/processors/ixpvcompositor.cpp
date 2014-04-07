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

#include "ixpvcompositor.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/renderdata.h"


#include "core/classification/simpletransferfunction.h"

#include "core/tools/quadrenderer.h"

namespace campvis {
    const std::string IxpvCompositor::loggerCat_ = "CAMPVis.modules.vis.IxpvCompositor";

    IxpvCompositor::IxpvCompositor(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_xRayImageId("XRayImageId", "X-Ray input image", "", DataNameProperty::READ)
        , p_3dSliceImageId("3DSliceImageId", "Slice input image rendered in 3D world space", "", DataNameProperty::READ)
        , p_drrFullImageId("DrrFullImageId", "Full DRR input image", "", DataNameProperty::READ)
        , p_drrClippedImageId("DrrClippedImageId", "Clipped DRR input image", "", DataNameProperty::READ)
        , p_targetImageId("TargetImageId", "Output Image", "", DataNameProperty::WRITE)
        , _shader(0)
    {
        addProperty(p_xRayImageId);
        addProperty(p_3dSliceImageId);
        addProperty(p_drrFullImageId);
        addProperty(p_drrClippedImageId);
        addProperty(p_targetImageId);

        addDecorator(new ProcessorDecoratorBackground());
        decoratePropertyCollection(this);
    }

    IxpvCompositor::~IxpvCompositor() {

    }

    void IxpvCompositor::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/ixpv/glsl/ixpvcompositor.frag", "");
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void IxpvCompositor::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void IxpvCompositor::updateResult(DataContainer& data) {
        ScopedTypedData<RenderData> xRayImage(data, p_xRayImageId.getValue());
        ScopedTypedData<RenderData> sliceImage(data, p_3dSliceImageId.getValue());
        ScopedTypedData<RenderData> drrFullImage(data, p_drrFullImageId.getValue());
        ScopedTypedData<RenderData> drrClippedImage(data, p_drrClippedImageId.getValue());

        if (xRayImage != 0 && sliceImage != 0 && drrFullImage != 0 && drrClippedImage != 0) {
            _shader->activate();
            tgt::TextureUnit xRayColorUnit, xRayDepthUnit, sliceColorUnit, sliceDepthUnit, drrFullUnit, drrClippedUnit;

            xRayImage->bind(_shader, xRayColorUnit, xRayDepthUnit, "_xRayColor", "_xRayDepth", "_xRayTexParams");
            sliceImage->bind(_shader, sliceColorUnit, sliceDepthUnit, "_sliceColor", "_sliceDepth", "_sliceTexParams");
            drrFullImage->bindColorTexture(_shader, drrFullUnit, "_drrFullColor", "_drrFullTexParams");
            drrClippedImage->bindColorTexture(_shader, drrClippedUnit, "_drrClippedColor", "_drrClippedTexParams");

            decorateRenderProlog(data, _shader);

            FramebufferActivationGuard fag(this);
            createAndAttachColorTexture();
            createAndAttachDepthTexture();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            QuadRdr.renderQuad();

            _shader->deactivate();
            tgt::TextureUnit::setZeroUnit();
            LGL_ERROR;

            data.addData(p_targetImageId.getValue(), new RenderData(_fbo));
        }
        else {
            LERROR("No suitable input images found.");
        }

        validate(INVALID_RESULT);
    }

}

