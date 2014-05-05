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

#include "sliceextractor.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"
#include "tgt/event/mouseevent.h"

#include "core/datastructures/facegeometry.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorbackground.h"

#include "core/classification/simpletransferfunction.h"

#include "core/tools/quadrenderer.h"

namespace campvis {
    static const GenericOption<SliceExtractor::SliceOrientation> sliceOrientationOptions[3] = {
        GenericOption<SliceExtractor::SliceOrientation>("z", "XY Plane", SliceExtractor::XY_PLANE),
        GenericOption<SliceExtractor::SliceOrientation>("y", "XZ Plane", SliceExtractor::XZ_PLANE),
        GenericOption<SliceExtractor::SliceOrientation>("x", "YZ Plane", SliceExtractor::YZ_PLANE)
    };

    const std::string SliceExtractor::loggerCat_ = "CAMPVis.modules.vis.SliceExtractor";

    SliceExtractor::SliceExtractor(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_sourceImageID("sourceImageID", "Input Image", "", DataNameProperty::READ)
        , p_geometryID("GeometryId", "Optional Input Geometry", "", DataNameProperty::READ)
        , p_targetImageID("targetImageID", "Output Image", "", DataNameProperty::WRITE)
        , p_sliceOrientation("SliceOrientation", "Slice Orientation", sliceOrientationOptions, 3)
        , p_xSliceNumber("XSliceNumber", "X Slice Number", 0, 0, 0)
        , p_xSliceColor("XSliceColor", "X Slice Color", tgt::vec4(1.f, 0.f, 0.f, 1.f), tgt::vec4(0.f), tgt::vec4(1.f))
        , p_ySliceNumber("YSliceNumber", "Y Slice Number", 0, 0, 0)
        , p_ySliceColor("YSliceColor", "Y Slice Color", tgt::vec4(0.f, 1.f, 0.f, 1.f), tgt::vec4(0.f), tgt::vec4(1.f))
        , p_zSliceNumber("ZSliceNumber", "Z Slice Number", 0, 0, 0)
        , p_zSliceColor("ZSliceColor", "Z Slice Color", tgt::vec4(0.f, 0.f, 1.f, 1.f), tgt::vec4(0.f), tgt::vec4(1.f))
        , p_transferFunction("transferFunction", "Transfer Function", new SimpleTransferFunction(256))
        , _shader(0)
        , _currentImage(nullptr)
        , _inScribbleMode(false)
    {
        addProperty(p_sourceImageID, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_geometryID);
        addProperty(p_targetImageID);
        addProperty(p_sliceOrientation);
        addProperty(p_xSliceNumber);
        addProperty(p_xSliceColor);
        addProperty(p_ySliceNumber);
        addProperty(p_ySliceColor);
        addProperty(p_zSliceNumber);
        addProperty(p_zSliceColor);
        addProperty(p_transferFunction);
    }

    SliceExtractor::~SliceExtractor() {

    }

    void SliceExtractor::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/vis/glsl/sliceextractor.frag", "");
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void SliceExtractor::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
        _currentImage = DataHandle(nullptr);
    }

    void SliceExtractor::updateResult(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_sourceImageID.getValue());
        ScopedTypedData<GeometryData> geometry(data, p_geometryID.getValue());

        if (img != 0) {
            if (img->getDimensionality() == 3) {
                tgt::vec3 imgSize(img->getSize());

                // current slices in texture coordinates
                tgt::vec3 sliceTexCoord = tgt::vec3(.5f + p_xSliceNumber.getValue(), .5f + p_ySliceNumber.getValue(), .5f + p_zSliceNumber.getValue()) / imgSize;
                // texture coordinate transformation matrix (will be configured later)
                tgt::mat4 texCoordsMatrix = tgt::mat4::zero;
                tgt::mat4 geometryPermutationMatrix = tgt::mat4::zero;
                tgt::mat4 geometryProjectionMatrix = tgt::mat4::identity;
                float foo = 0.f;

                float renderTargetRatio = static_cast<float>(getEffectiveViewportSize().x) / static_cast<float>(getEffectiveViewportSize().y);
                float sliceRatio = 1.f;

                switch (p_sliceOrientation.getValue()) {
                    case XY_PLANE:
                        // keep texture coordinates for x,y, shift z coordinates to slice value
                        texCoordsMatrix.t00 = 1.f;
                        texCoordsMatrix.t11 = 1.f;
                        texCoordsMatrix.t22 = 1.f;
                        texCoordsMatrix.t33 = 1.f;
                        texCoordsMatrix.t23 = sliceTexCoord.z;

                        geometryPermutationMatrix = tgt::mat4::identity;

                        // compute clip volume so that we only show the geometry at the current slice.
                        foo = (-2.f * static_cast<float>(p_zSliceNumber.getValue()) / imgSize.z) + 1.f;
                        geometryProjectionMatrix = tgt::mat4::createOrtho(-1.f, 1.f, 1.f, -1.f, foo - (1.f / p_zSliceNumber.getValue()), foo + (1.f / p_zSliceNumber.getValue()));

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

                        geometryPermutationMatrix.t00 = 1.f;
                        geometryPermutationMatrix.t12 = 1.f;
                        geometryPermutationMatrix.t21 = 1.f;
                        geometryPermutationMatrix.t33 = 1.f;

                        // compute clip volume so that we only show the geometry at the current slice.
                        foo = (-2.f * static_cast<float>(p_ySliceNumber.getValue()) / imgSize.y) + 1.f;
                        geometryProjectionMatrix = tgt::mat4::createOrtho(-1.f, 1.f, 1.f, -1.f, foo - (1.f / p_ySliceNumber.getValue()), foo + (.75f / p_zSliceNumber.getValue()));

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

                        geometryPermutationMatrix.t01 = 1.f; 
                        geometryPermutationMatrix.t12 = 1.f;
                        geometryPermutationMatrix.t20 = 1.f;
                        geometryPermutationMatrix.t33 = 1.f;

                        // compute clip volume so that we only show the geometry at the current slice.
                        foo = (-2.f * static_cast<float>(p_xSliceNumber.getValue()) / imgSize.x) + 1.f;
                        geometryProjectionMatrix = tgt::mat4::createOrtho(-1.f, 1.f, 1.f, -1.f, foo - (1.f / p_xSliceNumber.getValue()), foo + (.75f / p_xSliceNumber.getValue()));

                        sliceRatio = 
                              (static_cast<float>(imgSize.y) * img.getImageData()->getMappingInformation().getVoxelSize().y)
                            / (static_cast<float>(imgSize.z) * img.getImageData()->getMappingInformation().getVoxelSize().z);
                        break;
                }
                
                // configure model matrix so that slices are rendered with correct aspect posNormalized
                float ratioRatio = sliceRatio / renderTargetRatio;
                tgt::mat4 viewMatrix = (ratioRatio > 1) ? tgt::mat4::createScale(tgt::vec3(1.f, 1.f / ratioRatio, 1.f)) : tgt::mat4::createScale(tgt::vec3(ratioRatio, 1.f, 1.f));
                viewMatrix.t11 *= -1;
                tgt::mat4 originalViewMatrix = viewMatrix;

                // prepare OpenGL
                _shader->activate();
                tgt::TextureUnit inputUnit, tfUnit;
                img->bind(_shader, inputUnit);
                p_transferFunction.getTF()->bind(_shader, tfUnit);

                _shader->setUniform("_texCoordsMatrix", texCoordsMatrix);
                _shader->setUniform("_viewMatrix", viewMatrix);
                _shader->setUniform("_useTexturing", true);
                _shader->setUniform("_useSolidColor", true);

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

                    viewMatrix.t00 = 0.f;
                    viewMatrix.t03 = 2.f * sliceTexCoord.x - 1.f;
                    viewMatrix.t03 *= (ratioRatio > 1) ? 1.f : ratioRatio;
                    _shader->setUniform("_viewMatrix", viewMatrix);
                    _shader->setUniform("_color", p_xSliceColor.getValue());
                    QuadRdr.renderQuad(GL_LINE_STRIP);

                    viewMatrix.t00 = (ratioRatio > 1) ? 1.f : ratioRatio;
                    viewMatrix.t11 = 0.f;
                    viewMatrix.t03 = 0.f;
                    viewMatrix.t13 = 2.f * sliceTexCoord.y - 1.f;
                    viewMatrix.t13 *= (ratioRatio > 1) ? -1.f / ratioRatio : -1.f;
                    _shader->setUniform("_viewMatrix", viewMatrix);
                    _shader->setUniform("_color", p_ySliceColor.getValue());
                    QuadRdr.renderQuad(GL_LINE_STRIP);
                    break;

                case XZ_PLANE:
                    _shader->setUniform("_color", p_ySliceColor.getValue());
                    QuadRdr.renderQuad(GL_LINE_LOOP);

                    viewMatrix.t00 = 0.f;
                    viewMatrix.t03 = 2.f * sliceTexCoord.x - 1.f;
                    viewMatrix.t03 *= (ratioRatio > 1) ? 1.f : ratioRatio;
                    _shader->setUniform("_viewMatrix", viewMatrix);
                    _shader->setUniform("_color", p_xSliceColor.getValue());
                    QuadRdr.renderQuad(GL_LINE_STRIP);

                    viewMatrix.t00 = (ratioRatio > 1) ? 1.f : ratioRatio;
                    viewMatrix.t11 = 0.f;
                    viewMatrix.t03 = 0.f;
                    viewMatrix.t13 = -1.f * (2.f * sliceTexCoord.z - 1.f); // we swap the Y axis direction throughout the entire processor
                    viewMatrix.t13 *= (ratioRatio > 1) ? 1.f / ratioRatio : 1.f;
                    _shader->setUniform("_viewMatrix", viewMatrix);
                    _shader->setUniform("_color", p_zSliceColor.getValue());
                    QuadRdr.renderQuad(GL_LINE_STRIP);
                    break;

                case YZ_PLANE:
                    _shader->setUniform("_color", p_xSliceColor.getValue());
                    QuadRdr.renderQuad(GL_LINE_LOOP);

                    viewMatrix.t00 = 0.f;
                    viewMatrix.t03 = 2.f * sliceTexCoord.y - 1.f;
                    viewMatrix.t03 *= (ratioRatio > 1) ? 1.f : ratioRatio;
                    _shader->setUniform("_viewMatrix", viewMatrix);
                    _shader->setUniform("_color", p_ySliceColor.getValue());
                    QuadRdr.renderQuad(GL_LINE_STRIP);

                    viewMatrix.t00 = (ratioRatio > 1) ? 1.f : ratioRatio;
                    viewMatrix.t11 = 0.f;
                    viewMatrix.t03 = 0.f;
                    viewMatrix.t13 = -1.f * (2.f * sliceTexCoord.z - 1.f); // we swap the Y axis direction throughout the entire processor
                    viewMatrix.t13 *= (ratioRatio > 1) ? 1.f / ratioRatio : 1.f;
                    _shader->setUniform("_viewMatrix", viewMatrix);
                    _shader->setUniform("_color", p_zSliceColor.getValue());
                    QuadRdr.renderQuad(GL_LINE_STRIP);
                    break;
                }

                // render optional geometry
                if (geometry) {
                    // setup for geometry rendering
                    _shader->setUniform("_projectionMatrix", geometryProjectionMatrix);
                    _shader->setUniform("_viewMatrix", originalViewMatrix);
                    _shader->setUniform("_modelMatrix", geometryPermutationMatrix * tgt::mat4::createTranslation(tgt::vec3(-1.f, -1.f, -1.f)) * tgt::mat4::createScale(2.f / tgt::vec3(img.getImageData()->getSize())));
                    _shader->setUniform("_useTexturing", false);
                    _shader->setUniform("_useSolidColor", false);
                    glPointSize(4.f);

                    // render
                    geometry->render(GL_POINTS);

                    // recover
                    _shader->setUniform("_projectionMatrix", tgt::mat4::identity);
                    _shader->setUniform("_modelMatrix", tgt::mat4::identity);
                    _shader->setUniform("_useSolidColor", true);
                    glPointSize(1.f);
                }

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
        _currentImage = img.getDataHandle();
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

    void SliceExtractor::onEvent(tgt::Event* e) {
        // if there is nobody listening to the scribble signal, we can save the expensive computations...
        if (! s_scribblePainted.has_connections())
            return;

        // we need an image as reference
        if (_currentImage.getData() != nullptr) {
            if (const ImageData* id = static_cast<const ImageData*>(_currentImage.getData())) {
                const tgt::svec3& imageSize = id->getSize();

                // we only handle mouse events
                if (tgt::MouseEvent* me = dynamic_cast<tgt::MouseEvent*>(e)) {
                    // transform viewport coordinates to voxel coordinates
                    // this is the inverse computation performed by the shader during rendering
                    float renderTargetRatio = static_cast<float>(getEffectiveViewportSize().x) / static_cast<float>(getEffectiveViewportSize().y);
                    tgt::vec2 posNormalized = tgt::vec2(static_cast<float>(me->x()), static_cast<float>(me->y())) / tgt::vec2(_viewportSizeProperty->getValue());
                    tgt::vec3 imgSize(id->getSize());

                    float sliceRatio, ratioRatio;
                    tgt::svec3 voxel;

                    switch (p_sliceOrientation.getOptionValue()) {
                        case XY_PLANE:
                            sliceRatio = (imgSize.x * id->getMappingInformation().getVoxelSize().x) / (imgSize.y * id->getMappingInformation().getVoxelSize().y);
                            ratioRatio = sliceRatio / renderTargetRatio;
                            posNormalized -= (ratioRatio > 1) ? tgt::vec2(0.f, (1.f - (1.f / ratioRatio)) / 2.f) : tgt::vec2((1.f - ratioRatio) / 2.f, 0.f);
                            posNormalized *= (ratioRatio > 1) ? tgt::vec2(1.f, ratioRatio) : tgt::vec2(1.f / ratioRatio, 1.f);
                            posNormalized = tgt::clamp(posNormalized, 0.f, 1.f);

                            voxel = tgt::clamp(tgt::svec3(static_cast<size_t>(posNormalized.x * imageSize.x), static_cast<size_t>(posNormalized.y * imageSize.y), p_zSliceNumber.getValue()), tgt::svec3(0, 0, 0), imageSize);
                            break;
                        case XZ_PLANE:
                            sliceRatio = (imgSize.x * id->getMappingInformation().getVoxelSize().x) / (imgSize.z * id->getMappingInformation().getVoxelSize().z);
                            ratioRatio = sliceRatio / renderTargetRatio;
                            posNormalized -= (ratioRatio > 1) ? tgt::vec2(0.f, (1.f - (1.f / ratioRatio)) / 2.f) : tgt::vec2((1.f - ratioRatio) / 2.f, 0.f);
                            posNormalized *= (ratioRatio > 1) ? tgt::vec2(1.f, ratioRatio) : tgt::vec2(1.f / ratioRatio, 1.f);
                            posNormalized = tgt::clamp(posNormalized, 0.f, 1.f);

                            voxel = tgt::clamp(tgt::svec3(static_cast<size_t>(posNormalized.x * imageSize.x), p_ySliceNumber.getValue(), static_cast<size_t>(posNormalized.y * imageSize.z)), tgt::svec3(0, 0, 0), imageSize);
                            break;
                        case YZ_PLANE:
                            sliceRatio = (imgSize.y * id->getMappingInformation().getVoxelSize().y) / (imgSize.z * id->getMappingInformation().getVoxelSize().z);
                            ratioRatio = sliceRatio / renderTargetRatio;
                            posNormalized -= (ratioRatio > 1) ? tgt::vec2(0.f, (1.f - (1.f / ratioRatio)) / 2.f) : tgt::vec2((1.f - ratioRatio) / 2.f, 0.f);
                            posNormalized *= (ratioRatio > 1) ? tgt::vec2(1.f, ratioRatio) : tgt::vec2(1.f / ratioRatio, 1.f);
                            posNormalized = tgt::clamp(posNormalized, 0.f, 1.f);

                            voxel = tgt::clamp(tgt::svec3(p_xSliceNumber.getValue(), static_cast<size_t>(posNormalized.x * imageSize.y), static_cast<size_t>(posNormalized.y * imageSize.z)), tgt::svec3(0, 0, 0), imageSize);
                            break;
                    }

                    // okay, we computed the voxel under the mouse arrow, now we need to tell the outer world
                    if ((me->action() == tgt::MouseEvent::PRESSED) && (me->button() == tgt::MouseEvent::MOUSE_BUTTON_LEFT)) {
                        _inScribbleMode = true;
                        s_scribblePainted(voxel);
                    }
                    else if (_inScribbleMode && me->action() == tgt::MouseEvent::MOTION) {
                        s_scribblePainted(voxel);
                    }
                    else if (_inScribbleMode && me->action() == tgt::MouseEvent::RELEASED) {
                        _inScribbleMode = false;
                        return;
                    }
                }
            }
        }

    }

}
