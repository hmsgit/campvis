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

#include "slicerenderer2d.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"
#include "cgt/event/mouseevent.h"

#include "core/datastructures/facegeometry.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorbackground.h"

#include "core/classification/simpletransferfunction.h"

#include "core/tools/quadrenderer.h"

namespace campvis {

    const std::string SliceRenderer2D::loggerCat_ = "CAMPVis.modules.vis.SliceRenderer2D";

    SliceRenderer2D::SliceRenderer2D(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_sourceImageID("sourceImageID", "Input Image", "", DataNameProperty::READ)
        , p_targetImageID("targetImageID", "Output Image", "", DataNameProperty::WRITE)
        , p_invertXAxis("invert_x", "Invert X Axis", false)
        , p_invertYAxis("invert_y", "Invert Y Axis", false)
        , p_cropTop("crop_top", "Crop Top Pixels", 0, 0, 512)
        , p_cropLeft("crop_left", "Crop Left Pixels", 0, 0, 512)
        , p_cropBottom("crop_bottom", "Crop Bottom Pixels", 0, 0, 512)
        , p_cropRight("crop_right", "Crop Right Pixels", 0, 0, 512)
        , p_transferFunction("transferFunction", "Transfer Function", new SimpleTransferFunction(256))
        , _shader(0)
        , _lastImgSize(0,0,0)
    {
        addProperty(p_sourceImageID, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_targetImageID);
        addProperty(p_transferFunction);

        addProperty(p_cropTop, INVALID_RESULT);
        addProperty(p_cropLeft, INVALID_RESULT);
        addProperty(p_cropBottom, INVALID_RESULT);
        addProperty(p_cropRight, INVALID_RESULT);

        addProperty(p_invertXAxis, INVALID_RESULT);
        addProperty(p_invertYAxis, INVALID_RESULT);
    }

    SliceRenderer2D::~SliceRenderer2D() {

    }

    void SliceRenderer2D::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/vis/glsl/slicerenderer2d.frag", "");
    }

    void SliceRenderer2D::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void SliceRenderer2D::updateResult(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_sourceImageID.getValue());

        if (img != 0) {
            if (img->getDimensionality() == 2) {
                cgt::vec3 imgSize(img->getSize());
             
                float renderTargetRatio = static_cast<float>(getEffectiveViewportSize().x) / static_cast<float>(getEffectiveViewportSize().y);

                cgt::vec2 topLeft_px(static_cast<float>(p_cropLeft.getValue()), static_cast<float>(p_cropTop.getValue()));
                cgt::vec2 bottomRight_px(static_cast<float>(imgSize.x - p_cropRight.getValue()), static_cast<float>(imgSize.y - p_cropBottom.getValue()));
                cgt::vec2 croppedSize = bottomRight_px - topLeft_px;

                float sliceRatio =
                    (static_cast<float>(croppedSize.x) * img.getImageData()->getMappingInformation().getVoxelSize().x)
                    / (static_cast<float>(croppedSize.y) * img.getImageData()->getMappingInformation().getVoxelSize().y);
       
                // configure model matrix so that slices are rendered with correct aspect posNormalized
                float ratioRatio = sliceRatio / renderTargetRatio;
                cgt::mat4 viewMatrix = (ratioRatio > 1) ? cgt::mat4::createScale(cgt::vec3(1.f, 1.f / ratioRatio, 1.f)) : cgt::mat4::createScale(cgt::vec3(ratioRatio, 1.f, 1.f));
                viewMatrix.t11 *= -1;

                // prepare OpenGL
                _shader->activate();
                cgt::TextureUnit inputUnit, tfUnit;
                img->bind(_shader, inputUnit);
                p_transferFunction.getTF()->bind(_shader, tfUnit);

                if (p_invertXAxis.getValue())
                    viewMatrix *= cgt::mat4::createScale(cgt::vec3(-1, 1, 1));

                if (p_invertYAxis.getValue())
                    viewMatrix *= cgt::mat4::createScale(cgt::vec3(1, -1, 1));


                cgt::vec2 topLeft = topLeft_px / imgSize.xy();
                cgt::vec2 bottomRight = bottomRight_px / imgSize.xy();

                _shader->setUniform("_viewMatrix", viewMatrix);
                _shader->setUniform("_topLeft", topLeft);
                _shader->setUniform("_bottomRight", bottomRight);

                // render slice
                FramebufferActivationGuard fag(this);
                createAndAttachColorTexture();
                createAndAttachDepthTexture();
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                QuadRdr.renderQuad();


                _shader->deactivate();
                cgt::TextureUnit::setZeroUnit();

                data.addData(p_targetImageID.getValue(), new RenderData(_fbo));
            }
            else {
                LERROR("Input image must have dimensionality of 2.");
            }
        }
        else {
            LDEBUG("No suitable input image found.");
        }
    }

    void SliceRenderer2D::updateProperties(DataContainer& dc) {
        ScopedTypedData<ImageData> img(dc, p_sourceImageID.getValue());
        p_transferFunction.setImageHandle(img.getDataHandle());

        if (img != 0) {
            cgt::ivec3 size = img->getSize();

            /// NOTE: the setMaxValue calls create crashes, probably due to qt threading issues
            //          it is a lot more stable with this _lastImgSize check
            if (size.xy() != _lastImgSize.xy()) {
                p_cropTop.setMaxValue(size.y);
                p_cropBottom.setMaxValue(size.y);
                p_cropLeft.setMaxValue(size.x);
                p_cropRight.setMaxValue(size.x);
            
                _lastImgSize = size;
            }
        }
    }

}
