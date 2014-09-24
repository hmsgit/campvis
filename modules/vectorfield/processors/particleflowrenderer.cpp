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

#include "particleflowrenderer.h"

#include "tgt/buffer.h"
#include "tgt/tgt_math.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"
#include "tgt/vertexarrayobject.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/lightsourcedata.h"
#include "core/datastructures/geometrydatafactory.h"
#include "core/datastructures/renderdata.h"


namespace campvis {

    const std::string ParticleFlowRenderer::loggerCat_ = "CAMPVis.modules.classification.ParticleFlowRenderer";

    ParticleFlowRenderer::ParticleFlowRenderer(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_resetButton("ResetButton", "Reset")
        , p_stepButton("StepButton", "Step")
        , p_stepInt("StepInt", "Step", 0, 0, 10)
        , p_inputVectors("InputImage", "Input Image Vectors", "vectors", DataNameProperty::READ)
        , p_renderOutput("RenderOutput", "Output Image", "ParticleFlowRenderer.output", DataNameProperty::WRITE)
        , p_arrowSize("ArrowSize", "Arrow Size", 1.f, .001f, 5.f)
        , p_lenThresholdMin("LenThresholdMin", "Length Threshold Min", .001f, 0.f, 1000.f, 0.005f)
        , p_lenThresholdMax("LenThresholdMax", "Length Threshold Max", 10.f, 0.f, 10000.f, 10.f)
        , p_flowProfile1("FlowSpline1", "Flow Profile - Spline 1", 1.f, .0f, 2.f)
        , p_flowProfile2("FlowSpline2", "Flow Profile - Spline 2", 1.f, .0f, 2.f)
        , p_flowProfile3("FlowSpline3", "Flow Profile - Spline 3", 1.f, .0f, 2.f)
        , p_flowProfile4("FlowSpline4", "Flow Profile - Spline 4", 1.f, .0f, 2.f)
        , p_Time("time", "Time", 0, 0, 100)
        , p_enableShading("EnableShading", "Enable Shading", true)
        , p_lightId("LightId", "Input Light Source", "lightsource", DataNameProperty::READ)
        , p_camera("Camera", "Camera", tgt::Camera())
        , p_sliceNumber("SliceNumber", "Slice Number", 0, 0, 0)
        , _arrowGeometry(nullptr)
        , _shader(nullptr)
        , _positionBufferA(nullptr)
        , _positionBufferB(nullptr)
        , _velocityBufferA(nullptr)
        , _velocityBufferB(nullptr)
        , _startTimeBufferA(nullptr)
        , _startTimeBufferB(nullptr)
        , _initialPositionBuffer(nullptr)
        , _vaoA(nullptr)
        , _vaoB(nullptr)
    {
        addProperty(p_resetButton, INVALID_PROPERTIES);
        addProperty(p_stepButton, INVALID_RESULT | FIRST_FREE_TO_USE_INVALIDATION_LEVEL);
        addProperty(p_stepInt, INVALID_RESULT | FIRST_FREE_TO_USE_INVALIDATION_LEVEL);

        addProperty(p_inputVectors, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_renderOutput);
        addProperty(p_arrowSize);
        addProperty(p_lenThresholdMin);
        addProperty(p_lenThresholdMax);
        addProperty(p_camera);
        addProperty(p_sliceNumber);
        addProperty(p_Time);
        addProperty(p_flowProfile1);
        addProperty(p_flowProfile2);
        addProperty(p_flowProfile3);
        addProperty(p_flowProfile4);

        addProperty(p_enableShading, INVALID_RESULT | INVALID_PROPERTIES | INVALID_SHADER);
        addProperty(p_lightId);
    }

    ParticleFlowRenderer::~ParticleFlowRenderer() {

    }

    void ParticleFlowRenderer::init() {
        VisualizationProcessor::init();

        _shader = ShdrMgr.loadWithCustomGlslVersion("modules/vectorfield/glsl/particleflowrenderer.vert", "", "modules/vectorfield/glsl/particleflowrenderer.frag", generateGlslHeader(), "400");
        const char* outputNames[] = { "ex_Position", "ex_Velocity", "ex_StartTime" };
        glTransformFeedbackVaryings(_shader->getID(), 3, outputNames, GL_SEPARATE_ATTRIBS);
        _shader->linkProgram();
        tgtAssert(_shader->isLinked(), "Shader not linked!");
        LGL_ERROR;

        _arrowGeometry = GeometryDataFactory::createArrow(12, 0.35f, 0.05f, 0.09f);
    }

    void ParticleFlowRenderer::deinit() {
        ShdrMgr.dispose(_shader);

        delete _arrowGeometry;
        _arrowGeometry = 0;

        delete _positionBufferA;
        delete _positionBufferB;
        delete _velocityBufferA;
        delete _velocityBufferB;
        delete _startTimeBufferA;
        delete _startTimeBufferB;
        delete _initialPositionBuffer;

        VisualizationProcessor::deinit();
    }

    void ParticleFlowRenderer::updateResult(DataContainer& dataContainer) {
        const float frameLength = 0.1f;

        if (_initialPositionBuffer == 0 ) {
            LERROR("Transform-Feedback buffers not initialized.");
            return;
        }

        //GenericImageRepresentationLocal<float, 3>::ScopedRepresentation vectors(dataContainer, p_inputVectors.getValue());
        ImageRepresentationGL::ScopedRepresentation vectors(dataContainer, p_inputVectors.getValue());

        if (vectors) {
            ScopedTypedData<LightSourceData> light(dataContainer, p_lightId.getValue());

            if (p_enableShading.getValue() == false || light != nullptr) {
                const tgt::Camera& cam = p_camera.getValue();
                const tgt::svec3& imgSize = vectors->getSize();
                const int sliceNumber = p_sliceNumber.getValue();

                float scale = getTemporalFlowScaling((float)p_Time.getValue() / 100.f,
                    p_flowProfile1.getValue(),
                    p_flowProfile2.getValue(),
                    p_flowProfile3.getValue(),
                    p_flowProfile4.getValue());

                glEnable(GL_DEPTH_TEST);
                _shader->activate();

                _shader->setIgnoreUniformLocationError(true);
                _shader->setUniform("_viewportSizeRCP", 1.f / tgt::vec2(getEffectiveViewportSize()));
                _shader->setUniform("_projectionMatrix", cam.getProjectionMatrix());
                _shader->setUniform("_viewMatrix", cam.getViewMatrix());

                if (p_enableShading.getValue() && light != nullptr) {
                    light->bind(_shader, "_lightSource");
                }
                _shader->setIgnoreUniformLocationError(false);

                if (getInvalidationLevel() & FIRST_FREE_TO_USE_INVALIDATION_LEVEL) {
                    // stage 1: perform 1 step of particle simulation
                    _shader->selectSubroutine(tgt::ShaderObject::VERTEX_SHADER, "update");
                    _shader->setUniform("_time", _currentTime);
                    _shader->setUniform("_frameLength", frameLength);
                    _shader->setUniform("_lifetime", 10.f);

                    tgt::TextureUnit flowUnit;
                    vectors->bind(_shader, flowUnit, "_volume", "_volumeTextureParams");

                    glEnable(GL_RASTERIZER_DISCARD);
                    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, _feedback[_drawBuffer]);

                    glBeginTransformFeedback(GL_POINTS);
                        glBindVertexArray((_drawBuffer == 1) ? _vaoA->getId() : _vaoB->getId());
                        glDrawArrays(GL_POINTS, 0, _numParticles);
                    glEndTransformFeedback();

                    glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);
                    glDisable(GL_RASTERIZER_DISCARD);

                    _drawBuffer = 1 - _drawBuffer;
                    _currentTime += frameLength;
                    validate(FIRST_FREE_TO_USE_INVALIDATION_LEVEL);
                    LGL_ERROR;
                }

                // stage 2: render particles
                FramebufferActivationGuard fag(this);
                createAndAttachColorTexture();
                createAndAttachDepthTexture();
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                _shader->selectSubroutine(tgt::ShaderObject::VERTEX_SHADER, "render");
                glBindVertexArray((_drawBuffer == 1) ? _vaoA->getId() : _vaoB->getId());
                glPointSize(10.f);
                //glDrawTransformFeedback(GL_POINTS, _feedback[1-_drawBuffer]);
                glDrawArrays(GL_POINTS, 0, _numParticles);
                glPointSize(1.f);

                _shader->deactivate();
                glDisable(GL_DEPTH_TEST);
                LGL_ERROR;

                dataContainer.addData(p_renderOutput.getValue(), new RenderData(_fbo));
            }
            else {
                LDEBUG("Could not load light source from DataContainer.");
            }
        }
        else {
            LERROR("Could not find suitable input data.");
        }
    }

    void ParticleFlowRenderer::updateProperties(DataContainer& dataContainer) {
        p_lightId.setVisible(p_enableShading.getValue());

        GenericImageRepresentationLocal<float, 3>::ScopedRepresentation vectors(dataContainer, p_inputVectors.getValue());

        if (vectors) {
            // initialize buffers etc.
            initializeTransformFeedbackBuffers(vectors);
        }
        else {
            LERROR("No suitable input data found or size of images mismatch!");
        }
    }

    void ParticleFlowRenderer::updateShader() {
        _shader->setHeaders(generateGlslHeader());
        _shader->rebuild();
    }

    std::string ParticleFlowRenderer::generateGlslHeader() const {
        std::string toReturn;
        if (p_enableShading.getValue())
            toReturn += "#define ENABLE_SHADING\n";

        return toReturn;
    }

    void ParticleFlowRenderer::renderVectorArrow(const GenericImageRepresentationLocal<float, 3>* vectors, const tgt::vec3& position, float scale) {

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


    float ParticleFlowRenderer::getTemporalFlowScaling(float t, float Ct0, float Ct1, float Ct2, float Ct3)
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

    float ParticleFlowRenderer::evaluateCubicBSpline(float t)
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

    void ParticleFlowRenderer::initializeTransformFeedbackBuffers(const GenericImageRepresentationLocal<float, 3>* vectors) {
        delete _positionBufferA;
        delete _positionBufferB;
        delete _velocityBufferA;
        delete _velocityBufferB;
        delete _startTimeBufferA;
        delete _startTimeBufferB;
        delete _initialPositionBuffer;
        delete _vaoA;
        delete _vaoB;

        std::vector<tgt::vec3> initialPositions;
        std::vector<tgt::vec3> initialVelocities;
        std::vector<float> startTimes;

        tgt::vec3 imageSize = vectors->getSize();
        _numParticles = 100;

        for (GLuint i = 0; i < _numParticles; ++i) {
            tgt::vec3 position, velocity;

            do {
                position = tgt::vec3(generateRandomFloat(imageSize.x), generateRandomFloat(imageSize.y), generateRandomFloat(imageSize.z));
                velocity.x = vectors->getElementNormalizedLinear(position, 0);
                velocity.y = vectors->getElementNormalizedLinear(position, 1);
                velocity.z = vectors->getElementNormalizedLinear(position, 2);
            } while (velocity == tgt::vec3::zero);

            float startTime = generateRandomFloat(.1f);

            initialPositions.push_back(position);
            initialVelocities.push_back(velocity);
            startTimes.push_back(startTime);
        }

        _positionBufferA = new tgt::BufferObject(tgt::BufferObject::ARRAY_BUFFER, tgt::BufferObject::USAGE_DYNAMIC_COPY);
        _positionBufferB = new tgt::BufferObject(tgt::BufferObject::ARRAY_BUFFER, tgt::BufferObject::USAGE_DYNAMIC_COPY);
        _positionBufferA->data(&initialPositions.front(), _numParticles * sizeof(tgt::vec3), tgt::BufferObject::FLOAT, 3);
        _positionBufferB->data(&initialPositions.front(), _numParticles * sizeof(tgt::vec3), tgt::BufferObject::FLOAT, 3);

        _initialPositionBuffer = new tgt::BufferObject(tgt::BufferObject::ARRAY_BUFFER, tgt::BufferObject::USAGE_STREAM_READ);
        _initialPositionBuffer->data(&initialPositions.front(), _numParticles * sizeof(tgt::vec3), tgt::BufferObject::FLOAT, 3);

        _velocityBufferA = new tgt::BufferObject(tgt::BufferObject::ARRAY_BUFFER, tgt::BufferObject::USAGE_DYNAMIC_COPY);
        _velocityBufferB = new tgt::BufferObject(tgt::BufferObject::ARRAY_BUFFER, tgt::BufferObject::USAGE_DYNAMIC_COPY);
        _velocityBufferA->data(&initialVelocities.front(), _numParticles * sizeof(tgt::vec3), tgt::BufferObject::FLOAT, 3);
        _velocityBufferB->data(&initialVelocities.front(), _numParticles * sizeof(tgt::vec3), tgt::BufferObject::FLOAT, 3);

        _startTimeBufferA = new tgt::BufferObject(tgt::BufferObject::ARRAY_BUFFER, tgt::BufferObject::USAGE_DYNAMIC_COPY);
        _startTimeBufferB = new tgt::BufferObject(tgt::BufferObject::ARRAY_BUFFER, tgt::BufferObject::USAGE_DYNAMIC_COPY);
        _startTimeBufferA->data(&startTimes.front(), _numParticles * sizeof(float), tgt::BufferObject::FLOAT, 1);
        _startTimeBufferB->data(&startTimes.front(), _numParticles * sizeof(float), tgt::BufferObject::FLOAT, 1);


        glGenTransformFeedbacks(2, _feedback);

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, _feedback[0]);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, _positionBufferA->getId());
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, _velocityBufferA->getId());
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, _startTimeBufferA->getId());

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, _feedback[1]);
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 0, _positionBufferB->getId());
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 1, _velocityBufferB->getId());
        glBindBufferBase(GL_TRANSFORM_FEEDBACK_BUFFER, 2, _startTimeBufferB->getId());

        glBindTransformFeedback(GL_TRANSFORM_FEEDBACK, 0);

        _vaoA = new tgt::VertexArrayObject();
        LGL_ERROR;
        _vaoA->setVertexAttributePointer(0, _positionBufferA);
        LGL_ERROR;
        _vaoA->setVertexAttributePointer(1, _velocityBufferA);
        LGL_ERROR;
        _vaoA->setVertexAttributePointer(2, _startTimeBufferA);
        LGL_ERROR;
        _vaoA->setVertexAttributePointer(3, _initialPositionBuffer);
        LGL_ERROR;

        _vaoB = new tgt::VertexArrayObject();
        LGL_ERROR;
        _vaoB->setVertexAttributePointer(0, _positionBufferB);
        LGL_ERROR;
        _vaoB->setVertexAttributePointer(1, _velocityBufferB);
        LGL_ERROR;
        _vaoB->setVertexAttributePointer(2, _startTimeBufferB);
        LGL_ERROR;
        _vaoB->setVertexAttributePointer(3, _initialPositionBuffer);
        LGL_ERROR;

        _currentTime = 0.f;
        _drawBuffer = 0;
        invalidate(FIRST_FREE_TO_USE_INVALIDATION_LEVEL);
    }

    float ParticleFlowRenderer::generateRandomFloat(float max /*= 1.f*/) {
        std::default_random_engine::result_type val = _generator();
        double toReturn = static_cast<double>(val - _generator.min()) / static_cast<double>(_generator.max());
        return static_cast<float>(toReturn) * max;
    }

}
