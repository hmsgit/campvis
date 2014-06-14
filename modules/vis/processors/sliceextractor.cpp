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

    static const GenericOption<GLenum> renderOptions[4] = {
        GenericOption<GLenum>("points", "GL_POINTS", GL_POINTS),
        GenericOption<GLenum>("lines", "GL_LINES", GL_LINES),
        GenericOption<GLenum>("linestrip", "GL_LINE_STRIP", GL_LINE_STRIP),
        GenericOption<GLenum>("polygon", "GL_POLYGON", GL_POLYGON)
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
        , p_fitToWindow("FitToWindow", "Fit to Window", false)
        , p_scalingFactor("ScalingFactor", "Scaling Factor", 1.f, 0.f, 10.f, .1f, 2)
        , p_offset("Offset", "Offset", tgt::ivec2(0), tgt::ivec2(0), tgt::ivec2(100))
        , p_transferFunction("transferFunction", "Transfer Function", new SimpleTransferFunction(256))
        , p_geometryRenderMode("GeometryRenderMode", "Geometry Render Mode", renderOptions, 4)
        , p_geometryRenderSize("GeometryRenderSize", "Geometry Render Size", 4.f, 1.f, 10.f, 1.f, 1)
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
        addProperty(p_fitToWindow, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_scalingFactor);
        addProperty(p_offset);
        addProperty(p_transferFunction);
        addProperty(p_geometryRenderMode);
        addProperty(p_geometryRenderSize);
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

                tgt::ivec2 viewportSize = getEffectiveViewportSize();
                tgt::vec2 imageSize;
                float renderTargetRatio = static_cast<float>(viewportSize.x) / static_cast<float>(viewportSize.y);
                
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
                        geometryProjectionMatrix = tgt::mat4::createOrtho(-1.f, 1.f, 1.f, -1.f, foo - (.5f / imgSize.z), foo + (.5f / imgSize.z));

                        imageSize = tgt::vec2((static_cast<float>(imgSize.x) * img.getImageData()->getMappingInformation().getVoxelSize().x),
                                              (static_cast<float>(imgSize.y) * img.getImageData()->getMappingInformation().getVoxelSize().y));
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
                        geometryProjectionMatrix = tgt::mat4::createOrtho(-1.f, 1.f, 1.f, -1.f, foo - (.5f / imgSize.y), foo + (.5f / imgSize.y));

                        imageSize = tgt::vec2((static_cast<float>(imgSize.x) * img.getImageData()->getMappingInformation().getVoxelSize().x), 
                                              (static_cast<float>(imgSize.z) * img.getImageData()->getMappingInformation().getVoxelSize().z));
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
                        geometryProjectionMatrix = tgt::mat4::createOrtho(-1.f, 1.f, 1.f, -1.f, foo - (.5f / imgSize.x), foo + (.5f / imgSize.x));

                        imageSize = tgt::vec2((static_cast<float>(imgSize.y) * img.getImageData()->getMappingInformation().getVoxelSize().y),
                                              (static_cast<float>(imgSize.z) * img.getImageData()->getMappingInformation().getVoxelSize().z));
                        break;
                }
                
                // configure model matrix so that slices are rendered with correct aspect posNormalized
                float sliceRatio = imageSize.x / imageSize.y;
                float ratioRatio = sliceRatio / renderTargetRatio;
                tgt::mat4 viewMatrix;

                if (p_fitToWindow.getValue()) {
                    viewMatrix = (ratioRatio > 1) ? tgt::mat4::createScale(tgt::vec3(1.f, 1.f / ratioRatio, 1.f)) : tgt::mat4::createScale(tgt::vec3(ratioRatio, 1.f, 1.f));
                }
                else {
                    viewMatrix = tgt::mat4::createTranslation(tgt::vec3(2.f * p_offset.getValue().x * p_scalingFactor.getValue() / viewportSize.x, -2.f * p_offset.getValue().y * p_scalingFactor.getValue() / viewportSize.y, 0.f));
                    viewMatrix *= tgt::mat4::createScale(tgt::vec3(p_scalingFactor.getValue() * imageSize.x / viewportSize.x, p_scalingFactor.getValue() * imageSize.y / viewportSize.y, 1.f));
                }
                viewMatrix.t11 *= -1;

                // prepare OpenGL
                _shader->activate();
                tgt::TextureUnit inputUnit, tfUnit;
                img->bind(_shader, inputUnit);
                p_transferFunction.getTF()->bind(_shader, tfUnit);

                tgt::mat4 modelMatrix = tgt::mat4::identity;
                _shader->setUniform("_modelMatrix", modelMatrix);

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

                    modelMatrix.t00 = 0.f;
                    modelMatrix.t03 = 2.f * sliceTexCoord.x - 1.f;
                    _shader->setUniform("_modelMatrix", modelMatrix);
                    _shader->setUniform("_color", p_xSliceColor.getValue());
                    QuadRdr.renderQuad(GL_LINE_STRIP);

                    modelMatrix.t00 = 1.f;
                    modelMatrix.t11 = 0.f;
                    modelMatrix.t03 = 0.f;
                    modelMatrix.t13 = 2.f * sliceTexCoord.y - 1.f;
                    _shader->setUniform("_modelMatrix", modelMatrix);
                    _shader->setUniform("_color", p_ySliceColor.getValue());
                    QuadRdr.renderQuad(GL_LINE_STRIP);
                    break;

                case XZ_PLANE:
                    _shader->setUniform("_color", p_ySliceColor.getValue());
                    QuadRdr.renderQuad(GL_LINE_LOOP);

                    modelMatrix.t00 = 0.f;
                    modelMatrix.t03 = 2.f * sliceTexCoord.x - 1.f;
                    _shader->setUniform("_modelMatrix", modelMatrix);
                    _shader->setUniform("_color", p_xSliceColor.getValue());
                    QuadRdr.renderQuad(GL_LINE_STRIP);

                    modelMatrix.t00 = 1.f;
                    modelMatrix.t11 = 0.f;
                    modelMatrix.t03 = 0.f;
                    modelMatrix.t13 = 2.f * sliceTexCoord.z - 1.f;
                    _shader->setUniform("_modelMatrix", modelMatrix);
                    _shader->setUniform("_color", p_zSliceColor.getValue());
                    QuadRdr.renderQuad(GL_LINE_STRIP);
                    break;

                case YZ_PLANE:
                    _shader->setUniform("_color", p_xSliceColor.getValue());
                    QuadRdr.renderQuad(GL_LINE_LOOP);

                    modelMatrix.t00 = 0.f;
                    modelMatrix.t03 = 2.f * sliceTexCoord.y - 1.f;
                    _shader->setUniform("_modelMatrix", modelMatrix);
                    _shader->setUniform("_color", p_ySliceColor.getValue());
                    QuadRdr.renderQuad(GL_LINE_STRIP);

                    modelMatrix.t00 = (ratioRatio > 1) ? 1.f : ratioRatio;
                    modelMatrix.t11 = 0.f;
                    modelMatrix.t03 = 0.f;
                    modelMatrix.t13 = 2.f * sliceTexCoord.z - 1.f;
                    _shader->setUniform("_modelMatrix", modelMatrix);
                    _shader->setUniform("_color", p_zSliceColor.getValue());
                    QuadRdr.renderQuad(GL_LINE_STRIP);
                    break;
                }

                // render optional geometry
                if (geometry) {
                    // setup for geometry rendering
                    _shader->setUniform("_projectionMatrix", geometryProjectionMatrix);
                    _shader->setUniform("_viewMatrix", viewMatrix);
                    _shader->setUniform("_modelMatrix", geometryPermutationMatrix * tgt::mat4::createTranslation(tgt::vec3(-1.f, -1.f, -1.f)) * tgt::mat4::createScale(2.f / tgt::vec3(img.getImageData()->getSize())));
                    _shader->setUniform("_useTexturing", false);
                    _shader->setUniform("_useSolidColor", false);

                    glPointSize(p_geometryRenderSize.getValue());
                    glLineWidth(p_geometryRenderSize.getValue());

                    // render
                    geometry->render(p_geometryRenderMode.getValue());

                    // recover
                    _shader->setUniform("_projectionMatrix", tgt::mat4::identity);
                    _shader->setUniform("_modelMatrix", tgt::mat4::identity);
                    _shader->setUniform("_useSolidColor", true);
                    glPointSize(1.f);
                    glLineWidth(1.f);
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

            p_offset.setMinValue(tgt::ivec2(-tgt::max(imgSize)));
            p_offset.setMaxValue(tgt::ivec2( tgt::max(imgSize)));
        }

        p_scalingFactor.setVisible(! p_fitToWindow.getValue());
        p_offset.setVisible(! p_fitToWindow.getValue());

        validate(AbstractProcessor::INVALID_PROPERTIES);
    }

    void SliceExtractor::onEvent(tgt::Event* e) {
        // if there is nobody listening to the scribble signal, we can save the expensive computations...
        if (! s_scribblePainted.has_connections())
            return;

        // we need an image as reference
        if (_currentImage.getData() != nullptr) {
            if (const ImageData* id = static_cast<const ImageData*>(_currentImage.getData())) {

                // we only handle mouse events
                if (tgt::MouseEvent* me = dynamic_cast<tgt::MouseEvent*>(e)) {
                    // transform viewport coordinates to voxel coordinates
                    // this is the inverse computation performed by the shader during rendering
                    tgt::vec2 viewportSize = getEffectiveViewportSize();
                    float renderTargetRatio = viewportSize.x / viewportSize.y;
                    tgt::vec2 posNormalized = tgt::vec2(static_cast<float>(me->x()), static_cast<float>(me->y())) / tgt::vec2(_viewportSizeProperty->getValue());
                    tgt::vec3 imgSize(id->getSize());

                    tgt::vec2 imageSize;
                    switch (p_sliceOrientation.getOptionValue()) {
                        case XY_PLANE:
                            imageSize = tgt::vec2((static_cast<float>(imgSize.x) * id->getMappingInformation().getVoxelSize().x),
                                                  (static_cast<float>(imgSize.y) * id->getMappingInformation().getVoxelSize().y));
                            break;
                        case XZ_PLANE:
                            imageSize = tgt::vec2((static_cast<float>(imgSize.x) * id->getMappingInformation().getVoxelSize().x),
                                                  (static_cast<float>(imgSize.z) * id->getMappingInformation().getVoxelSize().z));
                            break;
                        case YZ_PLANE:
                            imageSize = tgt::vec2((static_cast<float>(imgSize.y) * id->getMappingInformation().getVoxelSize().y),
                                                  (static_cast<float>(imgSize.z) * id->getMappingInformation().getVoxelSize().z));
                            break;
                    }

                    if (p_fitToWindow.getValue()) {
                        float sliceRatio = imageSize.x / imageSize.y;
                        float ratioRatio = sliceRatio / renderTargetRatio;
                        posNormalized -= (ratioRatio > 1) ? tgt::vec2(0.f, (1.f - (1.f / ratioRatio)) / 2.f) : tgt::vec2((1.f - ratioRatio) / 2.f, 0.f);
                        posNormalized *= (ratioRatio > 1) ? tgt::vec2(1.f, ratioRatio) : tgt::vec2(1.f / ratioRatio, 1.f);
                    }
                    else {
                        posNormalized -= .5f;
                        posNormalized *= viewportSize / (imageSize * p_scalingFactor.getValue());
                        posNormalized -= tgt::vec2(p_offset.getValue()) / imageSize;
                        posNormalized += .5f;
                    }

                    tgt::vec3 voxel;
                    switch (p_sliceOrientation.getOptionValue()) {
                        case XY_PLANE:
                            voxel = tgt::vec3(posNormalized.x * imgSize.x, posNormalized.y * imgSize.y, static_cast<float>(p_zSliceNumber.getValue()));
                            break;
                        case XZ_PLANE:
                            voxel = tgt::vec3(posNormalized.x * imgSize.x, static_cast<float>(p_ySliceNumber.getValue()), posNormalized.y * imgSize.z);
                            break;
                        case YZ_PLANE:
                            voxel = tgt::vec3(static_cast<float>(p_xSliceNumber.getValue()), posNormalized.x * imgSize.y, posNormalized.y * imgSize.z);
                            break;
                    }

                    // okay, we computed the voxel under the mouse arrow, now we need to tell the outer world
                    if ((me->action() == tgt::MouseEvent::PRESSED) && (me->button() == tgt::MouseEvent::MOUSE_BUTTON_LEFT)) {
                        _inScribbleMode = true;
                        if (tgt::hand(tgt::greaterThanEqual(voxel, tgt::vec3(0.f))) && tgt::hand(tgt::lessThan(voxel, imgSize))) {
                            s_scribblePainted(voxel);
                        }
                    }
                    else if (_inScribbleMode && me->action() == tgt::MouseEvent::MOTION) {
                        if (tgt::hand(tgt::greaterThanEqual(voxel, tgt::vec3(0.f))) && tgt::hand(tgt::lessThan(voxel, imgSize))) {
                            s_scribblePainted(voxel);
                        }
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
