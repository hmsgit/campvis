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

#include "sliceextractor.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/facegeometry.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorbackground.h"

#include "core/classification/simpletransferfunction.h"

#include "core/tools/quadrenderer.h"

namespace campvis {
    static const GenericOption<SliceExtractor::SliceOrientation> compositingOptions[3] = {
        GenericOption<SliceExtractor::SliceOrientation>("z", "XY Plane", SliceExtractor::XY_PLANE),
        GenericOption<SliceExtractor::SliceOrientation>("y", "XZ Plane", SliceExtractor::XZ_PLANE),
        GenericOption<SliceExtractor::SliceOrientation>("x", "YZ Plane", SliceExtractor::YZ_PLANE)
    };

    const std::string SliceExtractor::loggerCat_ = "CAMPVis.modules.vis.SliceExtractor";

    SliceExtractor::SliceExtractor(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_sourceImageID("sourceImageID", "Input Image", "", DataNameProperty::READ, AbstractProcessor::INVALID_RESULT | AbstractProcessor::INVALID_PROPERTIES)
        , p_targetImageID("targetImageID", "Output Image", "", DataNameProperty::WRITE)
        , p_sliceOrientation("SliceOrientation", "Slice Orientation", compositingOptions, 3)
        , p_xSliceNumber("XSliceNumber", "X Slice Number", 0, 0, 0)
        , p_xSliceColor("XSliceColor", "X Slice Color", tgt::vec4(1.f, 0.f, 0.f, 1.f), tgt::vec4(0.f), tgt::vec4(1.f))
        , p_ySliceNumber("YSliceNumber", "Y Slice Number", 0, 0, 0)
        , p_ySliceColor("YSliceColor", "Y Slice Color", tgt::vec4(0.f, 1.f, 0.f, 1.f), tgt::vec4(0.f), tgt::vec4(1.f))
        , p_zSliceNumber("ZSliceNumber", "Z Slice Number", 0, 0, 0)
        , p_zSliceColor("ZSliceColor", "Z Slice Color", tgt::vec4(0.f, 0.f, 1.f, 1.f), tgt::vec4(0.f), tgt::vec4(1.f))
        , p_transferFunction("transferFunction", "Transfer Function", new SimpleTransferFunction(256))
        , _shader(0)
    {
        addProperty(&p_sourceImageID);
        addProperty(&p_targetImageID);
        addProperty(&p_sliceOrientation);
        addProperty(&p_xSliceNumber);
        addProperty(&p_xSliceColor);
        addProperty(&p_ySliceNumber);
        addProperty(&p_ySliceColor);
        addProperty(&p_zSliceNumber);
        addProperty(&p_zSliceColor);
        addProperty(&p_transferFunction);

        //addDecorator(new ProcessorDecoratorBackground());
        decoratePropertyCollection(this);
    }

    SliceExtractor::~SliceExtractor() {

    }

    void SliceExtractor::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/vis/glsl/sliceextractor.frag", "", false);
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void SliceExtractor::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void SliceExtractor::process(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_sourceImageID.getValue());

        if (img != 0) {
            if (img->getDimensionality() == 3) {
                tgt::vec3 imgSize(img->getSize());

                // current slices in texture coordinates
                tgt::vec3 sliceTexCoord = tgt::vec3(.5f + p_xSliceNumber.getValue(), .5f + p_ySliceNumber.getValue(), .5f + p_zSliceNumber.getValue()) / imgSize;
                // texture coordinate transformation matrix (will be configured later)
                tgt::mat4 texCoordsMatrix = tgt::mat4::zero;

                float renderTargetRatio = static_cast<float>(getEffectiveViewportSize().x) / static_cast<float>(getEffectiveViewportSize().y);
                float sliceRatio = 1.f;

                switch (p_sliceOrientation.getValue()) {
                    case XY_PLANE:
                        // keep texture coordinates for x,y, shift z coordinates to slice value
                        texCoordsMatrix.t00 = 1.f;
                        texCoordsMatrix.t11 = 1.f;
                        texCoordsMatrix.t22 = 1.f;
                        texCoordsMatrix.t23 = sliceTexCoord.z;
                        sliceRatio = 
                              (static_cast<float>(imgSize.x) * img.getImageData()->getMappingInformation().getVoxelSize().x)
                            / (static_cast<float>(imgSize.y) * img.getImageData()->getMappingInformation().getVoxelSize().y);
                        break;

                    case XZ_PLANE:
                        // permute y and z coordinates, shift y to slice 
                        texCoordsMatrix.t00 = 1.f;
                        texCoordsMatrix.t12 = 1.f;
                        texCoordsMatrix.t21 = 1.f;
                        texCoordsMatrix.t33 = 1.f;
                        texCoordsMatrix.t13 = sliceTexCoord.y;
                        sliceRatio = 
                               (static_cast<float>(imgSize.x) * img.getImageData()->getMappingInformation().getVoxelSize().x) 
                             / (static_cast<float>(imgSize.z) * img.getImageData()->getMappingInformation().getVoxelSize().z);
                        break;

                    case YZ_PLANE:
                        // permute x,y and z coordinates, shift x to slice
                        texCoordsMatrix.t02 = 1.f; 
                        texCoordsMatrix.t10 = 1.f;
                        texCoordsMatrix.t21 = 1.f;
                        texCoordsMatrix.t33 = 1.f;
                        texCoordsMatrix.t03 = sliceTexCoord.x;
                        sliceRatio = 
                              (static_cast<float>(imgSize.y) * img.getImageData()->getMappingInformation().getVoxelSize().y)
                            / (static_cast<float>(imgSize.z) * img.getImageData()->getMappingInformation().getVoxelSize().z);
                        break;
                }
                
                // configure model matrix so that slices are rendered with correct aspect ratio
                float ratioRatio = sliceRatio / renderTargetRatio;
                tgt::mat4 modelMatrix = (ratioRatio > 1) ? tgt::mat4::createScale(tgt::vec3(1.f, 1.f / ratioRatio, 1.f)) : tgt::mat4::createScale(tgt::vec3(ratioRatio, 1.f, 1.f));

                // prepare OpenGL
                _shader->activate();
                decorateRenderProlog(data, _shader);
                tgt::TextureUnit inputUnit, tfUnit;
                img->bind(_shader, inputUnit);
                p_transferFunction.getTF()->bind(_shader, tfUnit);

                _shader->setUniform("_texCoordsMatrix", texCoordsMatrix);
                _shader->setUniform("_modelMatrix", modelMatrix);
                _shader->setUniform("_useTexturing", true);

                // render slice
                FramebufferActivationGuard fag(this);
                createAndAttachColorTexture();
                createAndAttachDepthTexture();
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                QuadRdr.renderQuad();

                // render slice markers
                // for each slice render a bounding box (GL_LINE_LOOP) in slice color and horizontal/vertical
                // lines (GL_LINE_STRIP) as reference for the other axis-aligned slices
                glLineWidth(2.f);
                _shader->setUniform("_useTexturing", false);
                switch (p_sliceOrientation.getValue()) {
                    case XY_PLANE:
                        _shader->setUniform("_color", p_zSliceColor.getValue());
                        QuadRdr.renderQuad(GL_LINE_LOOP);

                        modelMatrix.t00 = 0.f;
                        modelMatrix.t03 = 2.f * sliceTexCoord.x - 1.f;
                        modelMatrix.t03 *= (ratioRatio > 1) ? 1.f : ratioRatio;
                        _shader->setUniform("_modelMatrix", modelMatrix);
                        _shader->setUniform("_color", p_xSliceColor.getValue());
                        QuadRdr.renderQuad(GL_LINE_STRIP);

                        modelMatrix.t00 = (ratioRatio > 1) ? 1.f : ratioRatio;
                        modelMatrix.t11 = 0.f;
                        modelMatrix.t03 = 0.f;
                        modelMatrix.t13 = 2.f * sliceTexCoord.y - 1.f;
                        modelMatrix.t13 *= (ratioRatio > 1) ? 1.f : ratioRatio;
                        _shader->setUniform("_modelMatrix", modelMatrix);
                        _shader->setUniform("_color", p_ySliceColor.getValue());
                        QuadRdr.renderQuad(GL_LINE_STRIP);
                        break;

                    case XZ_PLANE:
                        _shader->setUniform("_color", p_ySliceColor.getValue());
                        QuadRdr.renderQuad(GL_LINE_LOOP);

                        modelMatrix.t00 = 0.f;
                        modelMatrix.t03 = 2.f * sliceTexCoord.x - 1.f;
                        modelMatrix.t03 *= (ratioRatio > 1) ? 1.f : ratioRatio;
                        _shader->setUniform("_modelMatrix", modelMatrix);
                        _shader->setUniform("_color", p_xSliceColor.getValue());
                        QuadRdr.renderQuad(GL_LINE_STRIP);

                        modelMatrix.t00 = (ratioRatio > 1) ? 1.f : ratioRatio;
                        modelMatrix.t11 = 0.f;
                        modelMatrix.t03 = 0.f;
                        modelMatrix.t13 = 2.f * sliceTexCoord.z - 1.f;
                        modelMatrix.t13 *= (ratioRatio > 1) ? 1.f / ratioRatio : 1.f;
                        _shader->setUniform("_modelMatrix", modelMatrix);
                        _shader->setUniform("_color", p_zSliceColor.getValue());
                        QuadRdr.renderQuad(GL_LINE_STRIP);
                        break;

                    case YZ_PLANE:
                        _shader->setUniform("_color", p_xSliceColor.getValue());
                        QuadRdr.renderQuad(GL_LINE_LOOP);

                        modelMatrix.t00 = 0.f;
                        modelMatrix.t03 = 2.f * sliceTexCoord.y - 1.f;
                        modelMatrix.t03 *= (ratioRatio > 1) ? 1.f : ratioRatio;
                        _shader->setUniform("_modelMatrix", modelMatrix);
                        _shader->setUniform("_color", p_ySliceColor.getValue());
                        QuadRdr.renderQuad(GL_LINE_STRIP);

                        modelMatrix.t00 = (ratioRatio > 1) ? 1.f : ratioRatio;
                        modelMatrix.t11 = 0.f;
                        modelMatrix.t03 = 0.f;
                        modelMatrix.t13 = 2.f * sliceTexCoord.z - 1.f;
                        modelMatrix.t13 *= (ratioRatio > 1) ? 1.f / ratioRatio : 1.f;
                        _shader->setUniform("_modelMatrix", modelMatrix);
                        _shader->setUniform("_color", p_zSliceColor.getValue());
                        QuadRdr.renderQuad(GL_LINE_STRIP);
                        break;
                }

                decorateRenderEpilog(_shader);
                _shader->deactivate();
                tgt::TextureUnit::setZeroUnit();

                data.addData(p_targetImageID.getValue(), new RenderData(_fbo));
            }
            else {
                LERROR("Input image must have dimensionality of 3.");
            }
        }
        else {
            LERROR("No suitable input image found.");
        }

        validate(INVALID_RESULT);
    }

    void SliceExtractor::updateProperties(DataContainer& dc) {
        ScopedTypedData<ImageData> img(dc, p_sourceImageID.getValue());
        p_transferFunction.setImageHandle(img.getDataHandle());

        if (img != 0) {
            tgt::ivec3 imgSize = img->getSize();
            if (p_xSliceNumber.getMaxValue() != imgSize.x - 1){
                p_xSliceNumber.setMaxValue(imgSize.x - 1);
            }
            if (p_ySliceNumber.getMaxValue() != imgSize.y - 1){
                p_ySliceNumber.setMaxValue(imgSize.y - 1);
            }
            if (p_zSliceNumber.getMaxValue() != imgSize.z - 1){
                p_zSliceNumber.setMaxValue(imgSize.z - 1);
            }
        }

        validate(AbstractProcessor::INVALID_PROPERTIES);
    }

    void SliceExtractor::updateBorderGeometry() {
    }

}
