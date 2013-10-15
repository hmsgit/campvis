// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
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
        addProperty(&p_xRayImageId);
        addProperty(&p_3dSliceImageId);
        addProperty(&p_drrFullImageId);
        addProperty(&p_drrFullImageId);
        addProperty(&p_targetImageId);

        addDecorator(new ProcessorDecoratorBackground());
        decoratePropertyCollection(this);
    }

    IxpvCompositor::~IxpvCompositor() {

    }

    void IxpvCompositor::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/vis/glsl/ixpvcompositor.frag", "", false);
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void IxpvCompositor::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void IxpvCompositor::process(DataContainer& data) {
        ScopedTypedData<RenderData> xRayImage(data, p_xRayImageId.getValue());
        ScopedTypedData<RenderData> sliceImage(data, p_3dSliceImageId.getValue());
        ScopedTypedData<RenderData> drrFullImage(data, p_drrFullImageId.getValue());
        ScopedTypedData<RenderData> drrClippedImage(data, p_drrClippedImageId.getValue());

        if (xRayImage != 0 && sliceImage != 0 && drrFullImage != 0 && drrClippedImage != 0) {
            FramebufferActivationGuard fag(this);
            createAndAttachColorTexture();
            createAndAttachDepthTexture();

            _shader->activate();
            tgt::TextureUnit xRayColorUnit, xRayDepthUnit, sliceColorUnit, sliceDepthUnit, drrFullUnit, drrClippedUnit;

            xRayImage->bind(_shader, xRayColorUnit, xRayDepthUnit, "_xRayColor", "_xRayDepth", "_xRayTexParams");
            sliceImage->bind(_shader, sliceColorUnit, sliceDepthUnit, "_sliceColor", "_sliceDepth", "_sliceTexParams");
            drrFullImage->bindColorTexture(_shader, drrFullUnit, "_drrFullColor", "_drrFullTexParams");
            drrClippedImage->bindColorTexture(_shader, drrClippedUnit, "_drrClippedColor", "_drrClippedTexParams");

            decorateRenderProlog(data, _shader);

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

