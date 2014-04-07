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

#include "vectorfieldrenderer.h"

#include "tgt/tgt_math.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/geometrydatafactory.h"
#include "core/datastructures/renderdata.h"

#include "core/pipeline/processordecoratorshading.h"

namespace campvis {

    static const GenericOption<VectorFieldRenderer::SliceOrientation> sliceOrientationOptions[4] = {
        GenericOption<VectorFieldRenderer::SliceOrientation>("z", "XY Plane", VectorFieldRenderer::XY_PLANE),
        GenericOption<VectorFieldRenderer::SliceOrientation>("y", "XZ Plane", VectorFieldRenderer::XZ_PLANE),
        GenericOption<VectorFieldRenderer::SliceOrientation>("x", "YZ Plane", VectorFieldRenderer::YZ_PLANE),
        GenericOption<VectorFieldRenderer::SliceOrientation>("a", "XYZ Volume", VectorFieldRenderer::XYZ_VOLUME)
    };

    const std::string VectorFieldRenderer::loggerCat_ = "CAMPVis.modules.classification.VectorFieldRenderer";

    VectorFieldRenderer::VectorFieldRenderer(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputVectors("InputImage", "Input Image Vectors", "vectors", DataNameProperty::READ)
        , p_renderOutput("RenderOutput", "Output Image", "VectorFieldRenderer.output", DataNameProperty::WRITE)
        , p_arrowSize("ArrowSize", "Arrow Size", 1.f, .001f, 5.f)
        , p_lenThresholdMin("LenThresholdMin", "Length Threshold Min", .001f, 0.f, 1000.f, 0.005f)
        , p_lenThresholdMax("LenThresholdMax", "Length Threshold Max", 10.f, 0.f, 10000.f, 10.f)
        , p_flowProfile1("FlowSpline1", "Flow Profile - Spline 1", 1.f, .0f, 2.f)
        , p_flowProfile2("FlowSpline2", "Flow Profile - Spline 2", 1.f, .0f, 2.f)
        , p_flowProfile3("FlowSpline3", "Flow Profile - Spline 3", 1.f, .0f, 2.f)
        , p_flowProfile4("FlowSpline4", "Flow Profile - Spline 4", 1.f, .0f, 2.f)
        , p_Time("time", "Time", 0, 0, 100)
        , p_camera("Camera", "Camera", tgt::Camera())
        , p_sliceOrientation("SliceOrientation", "Slice Orientation", sliceOrientationOptions, 4)
        , p_sliceNumber("SliceNumber", "Slice Number", 0, 0, 0)
        , _arrowGeometry(0)
    {
        addDecorator(new ProcessorDecoratorShading());

        addProperty(p_inputVectors, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_renderOutput);
        addProperty(p_arrowSize);
        addProperty(p_lenThresholdMin);
        addProperty(p_lenThresholdMax);
        addProperty(p_camera);
        addProperty(p_sliceOrientation, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_sliceNumber);
        addProperty(p_Time);
        addProperty(p_flowProfile1);
        addProperty(p_flowProfile2);
        addProperty(p_flowProfile3);
        addProperty(p_flowProfile4);

        decoratePropertyCollection(this);
    }

    VectorFieldRenderer::~VectorFieldRenderer() {

    }

    void VectorFieldRenderer::init() {
        VisualizationProcessor::init();

        _shader = ShdrMgr.load("modules/vectorfield/glsl/vectorfieldrenderer.vert", "modules/vectorfield/glsl/vectorfieldrenderer.frag", generateGlslHeader());
        _arrowGeometry = GeometryDataFactory::createArrow(12, 0.35f, 0.05f, 0.09f);
    }

    void VectorFieldRenderer::deinit() {
        ShdrMgr.dispose(_shader);

        delete _arrowGeometry;
        _arrowGeometry = 0;

        VisualizationProcessor::deinit();
    }

    void VectorFieldRenderer::updateResult(DataContainer& dataContainer) {
        if (_arrowGeometry == 0 ) {
            LERROR("Error initializing arrow geometry.");
            return;
        }

        GenericImageRepresentationLocal<float, 3>::ScopedRepresentation vectors(dataContainer, p_inputVectors.getValue());

        if(vectors) {
            const tgt::Camera& cam = p_camera.getValue();
            const tgt::svec3& imgSize = vectors->getSize();
            const int sliceNumber = p_sliceNumber.getValue();

            glEnable(GL_DEPTH_TEST);
            _shader->activate();

            _shader->setIgnoreUniformLocationError(true);
            _shader->setUniform("_viewportSizeRCP", 1.f / tgt::vec2(getEffectiveViewportSize()));
            _shader->setUniform("_projectionMatrix", cam.getProjectionMatrix());
            _shader->setUniform("_viewMatrix", cam.getViewMatrix());
            decorateRenderProlog(dataContainer, _shader);

            FramebufferActivationGuard fag(this);
            createAndAttachColorTexture();
            createAndAttachDepthTexture();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            float scale = getTemporalFlowScaling((float)p_Time.getValue() / 100.f,
                p_flowProfile1.getValue(),
                p_flowProfile2.getValue(),
                p_flowProfile3.getValue(),
                p_flowProfile4.getValue());

            switch (p_sliceOrientation.getOptionValue()) {
                case XY_PLANE:
                    for (size_t x = 0; x < imgSize.x; ++x) {
                        for (size_t y = 0; y < imgSize.y; ++y) {
                            renderVectorArrow(vectors, tgt::ivec3(static_cast<int>(x), static_cast<int>(y), sliceNumber), scale);
                        }
                    }
                    break;
                case XZ_PLANE:
                    for (size_t x = 0; x < imgSize.x; ++x) {
                        for (size_t z = 0; z < imgSize.z; ++z) {
                            renderVectorArrow(vectors, tgt::ivec3(static_cast<int>(x), sliceNumber, static_cast<int>(z)), scale);
                        }
                    }
                    break;
                case YZ_PLANE:
                    for (size_t y = 0; y < imgSize.y; ++y) {
                        for (size_t z = 0; z < imgSize.z; ++z) {
                            renderVectorArrow(vectors, tgt::ivec3(sliceNumber, static_cast<int>(y), static_cast<int>(z)), scale);
                        }
                    }
                    break;
                case XYZ_VOLUME:
                    for (size_t x = 0; x < imgSize.x; ++x) {
                        for (size_t y = 0; y < imgSize.y; ++y) {
                            for (size_t z = 0; z < imgSize.z; ++z) {
                                renderVectorArrow(vectors, tgt::ivec3(static_cast<int>(x), static_cast<int>(y), static_cast<int>(z)), scale);
                            }
                        }
                    }
                    break;
            }

            decorateRenderEpilog(_shader);
            _shader->deactivate();
            glDisable(GL_DEPTH_TEST);

            dataContainer.addData(p_renderOutput.getValue(), new RenderData(_fbo));
        }
        else {
            LERROR("Could not find suitable input data.");
        }
        
        validate(INVALID_RESULT);
    }

    void VectorFieldRenderer::updateProperties(DataContainer& dataContainer) {

        GenericImageRepresentationLocal<float, 3>::ScopedRepresentation vectors(dataContainer, p_inputVectors.getValue());

        if(vectors) {
            switch (p_sliceOrientation.getOptionValue()) {
                case XY_PLANE:
                    p_sliceNumber.setMaxValue(static_cast<int>(vectors->getSize().z - 1));
                    break;
                case XZ_PLANE:
                    p_sliceNumber.setMaxValue(static_cast<int>(vectors->getSize().y - 1));
                    break;
                case YZ_PLANE:
                    p_sliceNumber.setMaxValue(static_cast<int>(vectors->getSize().x - 1));
                    break;
                case XYZ_VOLUME:
                    p_sliceNumber.setMaxValue(0);
            }
        }
        else {
            LERROR("No suitable input data found or size of images mismatch!");
        }
        validate(INVALID_PROPERTIES);
    }

    void VectorFieldRenderer::updateShader() {
        _shader->setHeaders(generateGlslHeader());
        _shader->rebuild();
        validate(INVALID_SHADER);
    }

    std::string VectorFieldRenderer::generateGlslHeader() const {
        std::string toReturn = getDecoratedHeader();

        return toReturn;
    }

    void VectorFieldRenderer::renderVectorArrow(const GenericImageRepresentationLocal<float, 3>* vectors, const tgt::vec3& position, float scale) {

        // avoid overflows
        if(position.x >= vectors->getSize().x || position.x < 0 ||
            position.y >= vectors->getSize().y || position.y < 0 ||
            position.z >= vectors->getSize().z || position.z < 0)
            return;

        // gather vector direction
        const tgt::vec3& dir = vectors->getElement(position);
        float len = tgt::length(dir);

        // threshold
        if(len < p_lenThresholdMin.getValue() || len > p_lenThresholdMax.getValue())
            return;

        tgt::vec3 up(0.f, 0.f, 1.f);
        tgt::vec3 dirNorm = tgt::normalize(dir);
        tgt::vec3 axis = tgt::cross(up, dirNorm);
        float dotPr = tgt::dot(up, dirNorm);
        tgt::mat4 rotationMatrix;
        if(abs(dotPr-1)<1.e-3f)
            rotationMatrix = tgt::mat4::identity;
        else if(abs(dotPr+1)<1.e-3f)
            rotationMatrix = tgt::mat4::createRotation(tgt::PIf, tgt::vec3(1.f, 0.f, 0.f));
        else {
            rotationMatrix = tgt::mat4::createRotation(acos(dotPr), tgt::normalize(axis));
        }

        const tgt::mat4& voxelToWorldMatrix = vectors->getParent()->getMappingInformation().getVoxelToWorldMatrix();

        // compute model matrix
        tgt::mat4 modelMatrix = voxelToWorldMatrix * tgt::mat4::createTranslation(position) * rotationMatrix *
            tgt::mat4::createScale(tgt::vec3(len * p_arrowSize.getValue())) * tgt::mat4::createScale(tgt::vec3(scale));

        // setup shader
        //_shader->setUniform("_color", tgt::vec4(dirNorm, 1.f));
        float color = (len - p_lenThresholdMin.getValue()) / (p_lenThresholdMax.getValue() - p_lenThresholdMin.getValue());
        _shader->setUniform("_color", tgt::vec4(1.f, 1-color, 1-color, 1.f));

        // render single ellipsoid
        _shader->setUniform("_modelMatrix", modelMatrix);
        _arrowGeometry->render(GL_TRIANGLE_STRIP);
    }


    float VectorFieldRenderer::getTemporalFlowScaling(float t, float Ct0, float Ct1, float Ct2, float Ct3)
    {
        const float halfPeriod = 0.5f;
        const float spacing = 0.25f;

        float St[4];

        for(int j = 0; j < 4; ++j) { // iterate over spline positions
            float splinePos = spacing * (j+1);

            // periodic alignment of samples -> contribution to all splines => dense sampling matrix
            if(t > splinePos + halfPeriod)
                t -= 1;
            else if(t < splinePos - halfPeriod)
                t += 1;

            float p = (splinePos - t) / spacing;
            St[j] = evaluateCubicBSpline(p);
        }

        return St[0]*Ct0 + St[1]*Ct1 + St[2]*Ct2 + St[3]*Ct3;
    }

    float VectorFieldRenderer::evaluateCubicBSpline(float t)
    {
        t += 2; // t is given zero-centered => shift peak from 2 to 0
        if(t <= 0 || t >= 4) 
            return 0;
        else if(t <= 1)
            return t*t*t / 6.0f;
        else if(t <= 2) {
            t -= 1;
            return (-3*t*t*t + 3*t*t + 3*t + 1) / 6.0f;
        }
        else if(t <= 3) {
            t -= 2;
            return (3*t*t*t - 6*t*t + 4) / 6.0f;
        }
        else {
            t -= 3;
            return (1-t)*(1-t)*(1-t) / 6.0f;
        }
    }

}
