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

#include "tensorglyphrenderer.h"

#include "cgt/cgt_math.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/lightsourcedata.h"
#include "core/datastructures/geometrydatafactory.h"
#include "core/datastructures/renderdata.h"

namespace campvis {

    static const GenericOption<TensorGlyphRenderer::GlyphType> glyphTypes[3] = {
        GenericOption<TensorGlyphRenderer::GlyphType>("ellipsoid", "Ellipsoid Glyph", TensorGlyphRenderer::ELLIPSOID),
        GenericOption<TensorGlyphRenderer::GlyphType>("cuboid", "Cuboid Glyph", TensorGlyphRenderer::CUBOID),
        GenericOption<TensorGlyphRenderer::GlyphType>("multi", "Multi Ellipsoid Glyph", TensorGlyphRenderer::MULTI)
    };

    static const GenericOption<TensorGlyphRenderer::SliceOrientation> sliceOrientationOptions[3] = {
        GenericOption<TensorGlyphRenderer::SliceOrientation>("z", "XY Plane", TensorGlyphRenderer::XY_PLANE),
        GenericOption<TensorGlyphRenderer::SliceOrientation>("y", "XZ Plane", TensorGlyphRenderer::XZ_PLANE),
        GenericOption<TensorGlyphRenderer::SliceOrientation>("x", "YZ Plane", TensorGlyphRenderer::YZ_PLANE)
    };

    const std::string TensorGlyphRenderer::loggerCat_ = "CAMPVis.modules.classification.TensorGlyphRenderer";

    TensorGlyphRenderer::TensorGlyphRenderer(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputEigenvalues("InputEigenvalues", "Input Eigenvalues Image", "eigenvalues", DataNameProperty::READ)
        , p_inputEigenvectors("InputEigenvectors", "Input Eigenvectors Image", "eigenvectors", DataNameProperty::READ)
        , p_renderOutput("RenderOutput", "Output Image", "TensorGlyphRenderer.output", DataNameProperty::WRITE)
        , p_glyphType("GlyphType", "Glyph Type to Render", glyphTypes, 3)
        , p_glyphSize("GlyphSize", "Glyph Size", 1.f, .1f, 5.f)
        , p_enableShading("EnableShading", "Enable Shading", true)
        , p_lightId("LightId", "Input Light Source", "lightsource", DataNameProperty::READ)
        , p_camera("Camera", "Camera", cgt::Camera())
        , p_sliceOrientation("SliceOrientation", "Slice Orientation", sliceOrientationOptions, 3)
        , p_sliceNumber("SliceNumber", "Slice Number", 0, 0, 0)
        , _ellipsoidGeometry(0)
        , _cubeGeometry(0)
    {
        addProperty(p_inputEigenvalues, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_inputEigenvectors, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_renderOutput);
        addProperty(p_glyphType);
        addProperty(p_glyphSize);

        addProperty(p_enableShading, INVALID_RESULT | INVALID_PROPERTIES | INVALID_SHADER);
        addProperty(p_lightId);

        addProperty(p_camera);
        addProperty(p_sliceOrientation, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_sliceNumber);
    }

    TensorGlyphRenderer::~TensorGlyphRenderer() {

    }

    void TensorGlyphRenderer::init() {
        VisualizationProcessor::init();

        _shader = ShdrMgr.load("modules/tensor/glsl/tensorglyphrenderer.vert", "modules/tensor/glsl/tensorglyphrenderer.frag", generateGlslHeader());
        _cubeGeometry = GeometryDataFactory::createCube(cgt::Bounds(cgt::vec3(-.5f), cgt::vec3(.5f)), cgt::Bounds(cgt::vec3(0.f), cgt::vec3(1.f)));
        _ellipsoidGeometry = GeometryDataFactory::createSphere(8, 16);
    }

    void TensorGlyphRenderer::deinit() {
        ShdrMgr.dispose(_shader);

        delete _ellipsoidGeometry;
        _ellipsoidGeometry = 0;
        delete _cubeGeometry;
        _cubeGeometry = 0;


        VisualizationProcessor::deinit();
    }

    void TensorGlyphRenderer::updateResult(DataContainer& dataContainer) {
        if (_cubeGeometry == 0 || _ellipsoidGeometry == 0) {
            LERROR("Error initializing glyph geometries.");
            return;
        }

        GenericImageRepresentationLocal<float, 3>::ScopedRepresentation evals(dataContainer, p_inputEigenvalues.getValue());
        GenericImageRepresentationLocal<float, 9>::ScopedRepresentation evecs(dataContainer, p_inputEigenvectors.getValue());

        if (evals && evecs) {
            if (evals->getSize() == evecs->getSize()) {
                ScopedTypedData<LightSourceData> light(dataContainer, p_lightId.getValue());

                if (p_enableShading.getValue() == false || light != nullptr) {
                    const cgt::Camera& cam = p_camera.getValue();
                    const cgt::svec3& imgSize = evals->getSize();

                    glEnable(GL_DEPTH_TEST);
                    _shader->activate();

                    _shader->setIgnoreUniformLocationError(true);
                    _shader->setUniform("_viewportSizeRCP", 1.f / cgt::vec2(getEffectiveViewportSize()));
                    _shader->setUniform("_projectionMatrix", cam.getProjectionMatrix());
                    _shader->setUniform("_viewMatrix", cam.getViewMatrix());

                    if (p_enableShading.getValue() && light != nullptr) {
                        light->bind(_shader, "_lightSource");
                    }

                    FramebufferActivationGuard fag(this);
                    createAndAttachColorTexture();
                    createAndAttachDepthTexture();
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    switch (p_sliceOrientation.getOptionValue()) {
                        case XY_PLANE:
                            for (size_t x = 0; x < imgSize.x; ++x) {
                                for (size_t y = 0; y < imgSize.y; ++y) {
                                    renderTensorGlyph(evals, evecs, cgt::ivec3(static_cast<int>(x), static_cast<int>(y), p_sliceNumber.getValue()));
                                }
                            }
                            break;
                        case XZ_PLANE:
                            for (size_t x = 0; x < imgSize.x; ++x) {
                                for (size_t z = 0; z < imgSize.z; ++z) {
                                    renderTensorGlyph(evals, evecs, cgt::ivec3(static_cast<int>(x), p_sliceNumber.getValue(), static_cast<int>(z)));
                                }
                            }
                            break;
                        case YZ_PLANE:
                            for (size_t y = 0; y < imgSize.y; ++y) {
                                for (size_t z = 0; z < imgSize.z; ++z) {
                                    renderTensorGlyph(evals, evecs, cgt::ivec3(p_sliceNumber.getValue(), static_cast<int>(y), static_cast<int>(z)));
                                }
                            }
                            break;
                    }


                    _shader->deactivate();
                    glDisable(GL_DEPTH_TEST);

                    dataContainer.addData(p_renderOutput.getValue(), new RenderData(_fbo));
                }
                else {
                    LDEBUG("Could not load light source from DataContainer.");
                }
            }
        }
        else {
            LERROR("Could not find suitable input data.");
        }
    }

    void TensorGlyphRenderer::updateProperties(DataContainer& dataContainer) {
        p_lightId.setVisible(p_enableShading.getValue());

        GenericImageRepresentationLocal<float, 3>::ScopedRepresentation evals(dataContainer, p_inputEigenvalues.getValue());
        GenericImageRepresentationLocal<float, 9>::ScopedRepresentation evecs(dataContainer, p_inputEigenvectors.getValue());

        if (evals && evecs) {
            if (evals->getSize() == evecs->getSize()) {
                switch (p_sliceOrientation.getOptionValue()) {
                    case XY_PLANE:
                        p_sliceNumber.setMaxValue(static_cast<int>(evals->getSize().z - 1));
                        break;
                    case XZ_PLANE:
                        p_sliceNumber.setMaxValue(static_cast<int>(evals->getSize().y - 1));
                        break;
                    case YZ_PLANE:
                        p_sliceNumber.setMaxValue(static_cast<int>(evals->getSize().x - 1));
                        break;
                }
            }
            else {
                LERROR("Size of eigenvalue image and eigenvector image mismatch!");
            }
        }
    }

    void TensorGlyphRenderer::updateShader() {
        _shader->setHeaders(generateGlslHeader());
        _shader->rebuild();
    }

    std::string TensorGlyphRenderer::generateGlslHeader() const {
        std::string toReturn;
        if (p_enableShading.getValue())
            toReturn += "#define ENABLE_SHADING\n";

        return toReturn;
    }

    void TensorGlyphRenderer::renderTensorGlyph(const GenericImageRepresentationLocal<float, 3>* evals, const GenericImageRepresentationLocal<float, 9>* evecs, const cgt::vec3& position) {
        /// minimum scale factor
        const float EPS = .1f;

        // gather value
        const cgt::vec3& eigenvalues = evals->getElement(position);
        const cgt::mat3& eigenvectors = evecs->getElement(position);
        if (eigenvalues == cgt::vec3::zero || eigenvectors == cgt::mat3::zero)
            return;

        // compute rotation matrix
        cgt::vec3 rotx = cgt::normalize(eigenvectors[0]);
        cgt::vec3 roty = cgt::normalize(eigenvectors[1]);
        cgt::vec3 rotz = cgt::normalize(eigenvectors[2]);
        cgt::mat4 rotationMatrix(rotx[0], rotx[1], rotx[2], 0.f,
            roty[0], roty[1], roty[2], 0.f,
            rotz[0], rotz[1], rotz[2], 0.f,
            0.f    , 0.f    , 0.f    , 1.f);

        float divScale = (1.f - 2.f*EPS)/(eigenvalues[0]);
        const cgt::mat4& voxelToWorldMatrix = evals->getParent()->getMappingInformation().getVoxelToWorldMatrix();

        // compute model matrix (without glyph-related transformation
        cgt::mat4 modelMatrix = voxelToWorldMatrix * cgt::mat4::createTranslation(position) * rotationMatrix * cgt::mat4::createScale(cgt::vec3(p_glyphSize.getValue()));

        // setup shader
        _shader->setUniform("_color", cgt::vec4(rotx, 1.f));

        switch (p_glyphType.getOptionValue()) {
            case CUBOID:
                // render single cuboid
                _shader->setUniform("_modelMatrix", modelMatrix * cgt::mat4::createScale(cgt::vec3((1.f - EPS), (EPS + divScale*eigenvalues[1]), (EPS + divScale*eigenvalues[2]))));
                _cubeGeometry->render(GL_POLYGON);
                break;

            case ELLIPSOID:
                // render single ellipsoid
                _shader->setUniform("_modelMatrix", modelMatrix * cgt::mat4::createScale(cgt::vec3((1.f - EPS), (EPS + divScale*eigenvalues[1]), (EPS + divScale*eigenvalues[2]))));
                _ellipsoidGeometry->render(GL_TRIANGLE_STRIP);
                break;

            case MULTI:
                // render three ellipsoids in different shapes
                _shader->setUniform("_modelMatrix", modelMatrix * cgt::mat4::createScale(cgt::vec3(divScale*eigenvalues[2], divScale*eigenvalues[2], divScale*eigenvalues[2])));
                _ellipsoidGeometry->render(GL_TRIANGLE_STRIP);
                _shader->setUniform("_modelMatrix", modelMatrix * cgt::mat4::createScale(cgt::vec3(divScale*eigenvalues[1], divScale*eigenvalues[1], EPS)));
                _ellipsoidGeometry->render(GL_TRIANGLE_STRIP);
                _shader->setUniform("_modelMatrix", modelMatrix * cgt::mat4::createScale(cgt::vec3(divScale*eigenvalues[0], EPS, EPS)));
                _ellipsoidGeometry->render(GL_TRIANGLE_STRIP);
                break;
        }
    }

}
