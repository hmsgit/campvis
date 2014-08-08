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

#include "slicerenderprocessor.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"
#include "tgt/event/mouseevent.h"

#include "core/datastructures/geometrydata.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/pipeline/processordecoratorbackground.h"

#include "core/tools/quadrenderer.h"

namespace campvis {
    static const GenericOption<SliceRenderProcessor::SliceOrientation> sliceOrientationOptions[3] = {
        GenericOption<SliceRenderProcessor::SliceOrientation>("z", "XY Plane", SliceRenderProcessor::XY_PLANE),
        GenericOption<SliceRenderProcessor::SliceOrientation>("y", "XZ Plane", SliceRenderProcessor::XZ_PLANE),
        GenericOption<SliceRenderProcessor::SliceOrientation>("x", "YZ Plane", SliceRenderProcessor::YZ_PLANE)
    };

    static const GenericOption<GLenum> renderOptions[4] = {
        GenericOption<GLenum>("points", "GL_POINTS", GL_POINTS),
        GenericOption<GLenum>("lines", "GL_LINES", GL_LINES),
        GenericOption<GLenum>("linestrip", "GL_LINE_STRIP", GL_LINE_STRIP),
        GenericOption<GLenum>("polygon", "GL_POLYGON", GL_POLYGON)
    };

    const std::string SliceRenderProcessor::loggerCat_ = "CAMPVis.modules.vis.SliceRenderProcessor";

    SliceRenderProcessor::SliceRenderProcessor(IVec2Property* viewportSizeProp, const std::string& fragmentShaderFileName, const std::string& customGlslVersion /*= ""*/)
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
        , p_renderCrosshair("RenderCrosshair", "Render Crosshair", true)
        , p_fitToWindow("FitToWindow", "Fit to Window", true)
        , p_scalingFactor("ScalingFactor", "Scaling Factor", 1.f, 0.f, 10.f, .1f, 2)
        , p_offset("Offset", "Offset", tgt::ivec2(0), tgt::ivec2(0), tgt::ivec2(100))
        , p_geometryRenderMode("GeometryRenderMode", "Geometry Render Mode", renderOptions, 4)
        , p_geometryRenderSize("GeometryRenderSize", "Geometry Render Size", 4.f, 1.f, 10.f, 1.f, 1)
        , _shader(nullptr)
        , _currentImage(nullptr)
        , _inScribbleMode(false)
        , _fragmentShaderFilename(fragmentShaderFileName)
        , _customGlslVersion(customGlslVersion)
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
        addProperty(p_renderCrosshair);
        addProperty(p_fitToWindow, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_scalingFactor);
        addProperty(p_offset);
        addProperty(p_geometryRenderMode);
        addProperty(p_geometryRenderSize);
    }

    SliceRenderProcessor::~SliceRenderProcessor() {

    }

    void SliceRenderProcessor::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadWithCustomGlslVersion("core/glsl/passthrough.vert", "", _fragmentShaderFilename, getGlslHeader(), _customGlslVersion);
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void SliceRenderProcessor::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
        _currentImage = DataHandle(nullptr);
    }

    void SliceRenderProcessor::updateResult(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_sourceImageID.getValue());

        if (img != 0) {
            if (img->getDimensionality() == 3) {
                setupMatrices(img);
                renderImageImpl(data, img);
            }
            else {
                LERROR("Input image must have dimensionality of 3.");
            }
        }
        else {
            LDEBUG("No suitable input image found.");
        }
    }

    void SliceRenderProcessor::updateProperties(DataContainer& dc) {
        ScopedTypedData<ImageData> img(dc, p_sourceImageID.getValue());
        _currentImage = img.getDataHandle();

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
    }

    void SliceRenderProcessor::updateShader() {
        std::string header = getGlslHeader();
        _shader->setHeaders(header);
        _shader->rebuild();
    }

    void SliceRenderProcessor::onEvent(tgt::Event* e) {
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

                    tgt::vec2 imageSize(0.f);
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

    void SliceRenderProcessor::setupMatrices(const ImageRepresentationGL::ScopedRepresentation& img) {
        tgt::vec3 imgSize(img->getSize());

        // current slices in texture coordinates
        tgt::vec3 sliceTexCoord = tgt::vec3(.5f + p_xSliceNumber.getValue(), .5f + p_ySliceNumber.getValue(), .5f + p_zSliceNumber.getValue()) / imgSize;
        float clip = 0.f;

        tgt::ivec2 viewportSize = getEffectiveViewportSize();
        tgt::vec2 imageSize(0.f);
        float renderTargetRatio = static_cast<float>(viewportSize.x) / static_cast<float>(viewportSize.y);

        _texCoordMatrix = tgt::mat4::zero;
        _geometryModelMatrix = tgt::mat4::identity;
        switch (p_sliceOrientation.getValue()) {
            case XY_PLANE:
                // keep texture coordinates for x,y, shift z coordinates to slice value
                _texCoordMatrix.t00 = 1.f;
                _texCoordMatrix.t11 = 1.f;
                _texCoordMatrix.t22 = 1.f;
                _texCoordMatrix.t33 = 1.f;
                _texCoordMatrix.t23 = sliceTexCoord.z;

                // compute clip volume so that we only show the geometry at the current slice.
                clip = (-2.f * static_cast<float>(p_zSliceNumber.getValue()) / imgSize.z) + 1.f;
                _geometryProjectionMatrix = tgt::mat4::createOrtho(-1.f, 1.f, 1.f, -1.f, clip - (.5f / imgSize.z), clip + (.5f / imgSize.z));

                imageSize = tgt::vec2((static_cast<float>(imgSize.x) * img.getImageData()->getMappingInformation().getVoxelSize().x),
                    (static_cast<float>(imgSize.y) * img.getImageData()->getMappingInformation().getVoxelSize().y));
                break;

            case XZ_PLANE:
                // permute y and z coordinates, shift y to slice 
                _texCoordMatrix.t00 = 1.f;
                _texCoordMatrix.t12 = 1.f;
                _texCoordMatrix.t21 = 1.f;
                _texCoordMatrix.t33 = 1.f;
                _texCoordMatrix.t13 = sliceTexCoord.y;

                _geometryModelMatrix.t00 = 1.f;
                _geometryModelMatrix.t12 = 1.f;
                _geometryModelMatrix.t21 = 1.f;
                _geometryModelMatrix.t33 = 1.f;

                // compute clip volume so that we only show the geometry at the current slice.
                clip = (-2.f * static_cast<float>(p_ySliceNumber.getValue()) / imgSize.y) + 1.f;
                _geometryProjectionMatrix = tgt::mat4::createOrtho(-1.f, 1.f, 1.f, -1.f, clip - (.5f / imgSize.y), clip + (.5f / imgSize.y));

                imageSize = tgt::vec2((static_cast<float>(imgSize.x) * img.getImageData()->getMappingInformation().getVoxelSize().x), 
                    (static_cast<float>(imgSize.z) * img.getImageData()->getMappingInformation().getVoxelSize().z));
                break;

            case YZ_PLANE:
                // permute x,y and z coordinates, shift x to slice
                _texCoordMatrix.t02 = 1.f; 
                _texCoordMatrix.t10 = 1.f;
                _texCoordMatrix.t21 = 1.f;
                _texCoordMatrix.t33 = 1.f;
                _texCoordMatrix.t03 = sliceTexCoord.x;

                _geometryModelMatrix.t01 = 1.f; 
                _geometryModelMatrix.t12 = 1.f;
                _geometryModelMatrix.t20 = 1.f;
                _geometryModelMatrix.t33 = 1.f;

                // compute clip volume so that we only show the geometry at the current slice.
                clip = (-2.f * static_cast<float>(p_xSliceNumber.getValue()) / imgSize.x) + 1.f;
                _geometryProjectionMatrix = tgt::mat4::createOrtho(-1.f, 1.f, 1.f, -1.f, clip - (.5f / imgSize.x), clip + (.5f / imgSize.x));

                imageSize = tgt::vec2((static_cast<float>(imgSize.y) * img.getImageData()->getMappingInformation().getVoxelSize().y),
                    (static_cast<float>(imgSize.z) * img.getImageData()->getMappingInformation().getVoxelSize().z));
                break;
        }

        // configure model matrix so that slices are rendered with correct aspect posNormalized
        float sliceRatio = imageSize.x / imageSize.y;
        float ratioRatio = sliceRatio / renderTargetRatio;

        if (p_fitToWindow.getValue()) {
            _viewMatrix = (ratioRatio > 1) ? tgt::mat4::createScale(tgt::vec3(1.f, 1.f / ratioRatio, 1.f)) : tgt::mat4::createScale(tgt::vec3(ratioRatio, 1.f, 1.f));
        }
        else {
            _viewMatrix = tgt::mat4::createTranslation(tgt::vec3(2.f * p_offset.getValue().x * p_scalingFactor.getValue() / viewportSize.x, -2.f * p_offset.getValue().y * p_scalingFactor.getValue() / viewportSize.y, 0.f));
            _viewMatrix *= tgt::mat4::createScale(tgt::vec3(p_scalingFactor.getValue() * imageSize.x / viewportSize.x, p_scalingFactor.getValue() * imageSize.y / viewportSize.y, 1.f));
        }
        _viewMatrix.t11 *= -1;
    }

    void SliceRenderProcessor::renderCrosshair(const ImageRepresentationGL::ScopedRepresentation& img) {

        // render slice markers
        // for each slice render a bounding box (GL_LINE_LOOP) in slice color and horizontal/vertical
        // lines (GL_LINE_STRIP) as reference for the other axis-aligned slices
        glLineWidth(2.f);
        _shader->setUniform("_useTexturing", false);

        tgt::mat4 modelMatrix = tgt::mat4::identity;
        tgt::vec3 sliceTexCoord = tgt::vec3(.5f + p_xSliceNumber.getValue(), .5f + p_ySliceNumber.getValue(), .5f + p_zSliceNumber.getValue()) / tgt::vec3(img->getSize());

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

                modelMatrix.t00 = 1.f; //(ratioRatio > 1) ? 1.f : ratioRatio;
                modelMatrix.t11 = 0.f;
                modelMatrix.t03 = 0.f;
                modelMatrix.t13 = 2.f * sliceTexCoord.z - 1.f;
                _shader->setUniform("_modelMatrix", modelMatrix);
                _shader->setUniform("_color", p_zSliceColor.getValue());
                QuadRdr.renderQuad(GL_LINE_STRIP);
                break;
        }

    }

    void SliceRenderProcessor::renderGeometry(DataContainer& dataContainer, const ImageRepresentationGL::ScopedRepresentation& img) {
        ScopedTypedData<GeometryData> geometry(dataContainer, p_geometryID.getValue());

        // render optional geometry
        if (geometry) {
            // setup for geometry rendering
            _shader->setUniform("_projectionMatrix", _geometryProjectionMatrix);
            _shader->setUniform("_viewMatrix", _viewMatrix);
            _shader->setUniform("_modelMatrix", _geometryModelMatrix * tgt::mat4::createTranslation(tgt::vec3(-1.f, -1.f, -1.f)) * tgt::mat4::createScale(2.f / tgt::vec3(img->getSize())));
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
    }

    std::string SliceRenderProcessor::getGlslHeader() {
        return "";
    }

}
