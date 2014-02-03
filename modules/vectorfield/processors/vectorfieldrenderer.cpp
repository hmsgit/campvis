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

    static const GenericOption<VectorFieldRenderer::SliceOrientation> sliceOrientationOptions[3] = {
        GenericOption<VectorFieldRenderer::SliceOrientation>("z", "XY Plane", VectorFieldRenderer::XY_PLANE),
        GenericOption<VectorFieldRenderer::SliceOrientation>("y", "XZ Plane", VectorFieldRenderer::XZ_PLANE),
        GenericOption<VectorFieldRenderer::SliceOrientation>("x", "YZ Plane", VectorFieldRenderer::YZ_PLANE)
    };

    const std::string VectorFieldRenderer::loggerCat_ = "CAMPVis.modules.classification.VectorFieldRenderer";

    VectorFieldRenderer::VectorFieldRenderer(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_renderOutput("RenderOutput", "Output Image", "VectorFieldRenderer.output", DataNameProperty::WRITE)
		, p_inputVectorX("InputImageX", "Input Image Vector X", "vectorX", DataNameProperty::READ, INVALID_RESULT | INVALID_PROPERTIES)
		, p_inputVectorY("InputImageY", "Input Image Vector Y", "vectorY", DataNameProperty::READ, INVALID_RESULT | INVALID_PROPERTIES)
		, p_inputVectorZ("InputImageZ", "Input Image Vector Z", "vectorZ", DataNameProperty::READ, INVALID_RESULT | INVALID_PROPERTIES)
        , p_arrowSize("ArrowSize", "Arrow Size", 1.f, .1f, 5.f)
		, p_lenThresholdMin("LenThresholdMin", "Length Threshold Min", .001f, 0.f, 1.f, 0.005f)
		, p_lenThresholdMax("LenThresholdMax", "Length Threshold Max", 10.f, 0.f, 10000.f, 10.f)
        , p_camera("Camera", "Camera", tgt::Camera())
        , p_sliceOrientation("SliceOrientation", "Slice Orientation", sliceOrientationOptions, 3, INVALID_RESULT | INVALID_PROPERTIES)
        , p_sliceNumber("SliceNumber", "Slice Number", 0, 0, 0)
        , _arrowGeometry(0)
    {
        addDecorator(new ProcessorDecoratorShading());

		addProperty(&p_inputVectorX);
		addProperty(&p_inputVectorY);
		addProperty(&p_inputVectorZ);
        addProperty(&p_renderOutput);
        addProperty(&p_arrowSize);
		addProperty(&p_lenThresholdMin);
		addProperty(&p_lenThresholdMax);
        addProperty(&p_camera);
        addProperty(&p_sliceOrientation);
        addProperty(&p_sliceNumber);

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

		GenericImageRepresentationLocal<float, 1>::ScopedRepresentation vectorX(dataContainer, p_inputVectorX.getValue());
		GenericImageRepresentationLocal<float, 1>::ScopedRepresentation vectorY(dataContainer, p_inputVectorY.getValue());
		GenericImageRepresentationLocal<float, 1>::ScopedRepresentation vectorZ(dataContainer, p_inputVectorZ.getValue());

        if (vectorX && vectorY && vectorZ && 
			vectorX->getSize() == vectorY->getSize() &&
			vectorX->getSize() == vectorZ->getSize()) {

            const tgt::Camera& cam = p_camera.getValue();
            const tgt::svec3& imgSize = vectorX->getSize();
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

            switch (p_sliceOrientation.getOptionValue()) {
                case XY_PLANE:
                    for (size_t x = 0; x < imgSize.x; ++x) {
                        for (size_t y = 0; y < imgSize.y; ++y) {
                            renderVectorArrow(vectorX, vectorY, vectorZ, tgt::ivec3(static_cast<int>(x), static_cast<int>(y), sliceNumber));
                        }
                    }
                    break;
                case XZ_PLANE:
                    for (size_t x = 0; x < imgSize.x; ++x) {
                        for (size_t z = 0; z < imgSize.z; ++z) {
                            renderVectorArrow(vectorX, vectorY, vectorZ, tgt::ivec3(static_cast<int>(x), sliceNumber, static_cast<int>(z)));
                        }
                    }
                    break;
                case YZ_PLANE:
                    for (size_t y = 0; y < imgSize.y; ++y) {
                        for (size_t z = 0; z < imgSize.z; ++z) {
                            renderVectorArrow(vectorX, vectorY, vectorZ, tgt::ivec3(sliceNumber, static_cast<int>(y), static_cast<int>(z)));
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

		GenericImageRepresentationLocal<float, 1>::ScopedRepresentation vectorX(dataContainer, p_inputVectorX.getValue());
		GenericImageRepresentationLocal<float, 1>::ScopedRepresentation vectorY(dataContainer, p_inputVectorY.getValue());
		GenericImageRepresentationLocal<float, 1>::ScopedRepresentation vectorZ(dataContainer, p_inputVectorZ.getValue());

        if (vectorX && vectorY && vectorZ && 
			vectorX->getSize() == vectorY->getSize() &&
			vectorX->getSize() == vectorZ->getSize()) {

			switch (p_sliceOrientation.getOptionValue()) {
                case XY_PLANE:
                    p_sliceNumber.setMaxValue(static_cast<int>(vectorX->getSize().z - 1));
                    break;
                case XZ_PLANE:
                    p_sliceNumber.setMaxValue(static_cast<int>(vectorX->getSize().y - 1));
                    break;
                case YZ_PLANE:
                    p_sliceNumber.setMaxValue(static_cast<int>(vectorX->getSize().x - 1));
                    break;
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

    void VectorFieldRenderer::renderVectorArrow(const GenericImageRepresentationLocal<float, 1>* vectorX,
			const GenericImageRepresentationLocal<float, 1>* vectorY,
			const GenericImageRepresentationLocal<float, 1>* vectorZ,
			const tgt::vec3& position) {

        /// minimum scale factor
        const float EPS = .1f;

        // gather vector direction
		tgt::vec3 dir(vectorX->getElement(position), vectorY->getElement(position), vectorZ->getElement(position));
		float len = tgt::length(dir);

		// threshold
		if(len < p_lenThresholdMin.getValue() || len > p_lenThresholdMax.getValue())
			return;

		tgt::vec3 up(0.f, 0.f, 1.f);
		tgt::vec3 dirNorm = tgt::normalize(dir);
		tgt::vec3 axis = tgt::cross(up, dirNorm);
		float aCos = tgt::dot(up, dirNorm);
		float aCosI = 1 - aCos;
		float aSin = sin(acos(aCos));

        // compute rotation matrix
        tgt::mat4 rotationMatrix(
			axis.x*axis.x*aCosI + aCos, axis.x*axis.y*aCosI - axis.z*aSin, axis.x*axis.z*aCosI + axis.y*aSin, 0.f,
            axis.x*axis.y+aCosI + axis.z*aSin, axis.y*axis.y*aCosI + aCos, axis.y*axis.z*aCosI - axis.x*aSin, 0.f,
            axis.x*axis.z*aCosI - axis.y*aSin, axis.y*axis.z*aCosI + axis.x*aSin, axis.z*axis.z*aCosI + aCos, 0.f,
            0.f    , 0.f    , 0.f    , 1.f);

        const tgt::mat4& voxelToWorldMatrix = vectorX->getParent()->getMappingInformation().getVoxelToWorldMatrix();

        // compute model matrix
        tgt::mat4 modelMatrix = voxelToWorldMatrix * tgt::mat4::createTranslation(position) * rotationMatrix * 
			tgt::mat4::createScale(tgt::vec3(len * p_arrowSize.getValue()));

        // setup shader
        _shader->setUniform("_color", tgt::vec4(dirNorm, 1.f));

		// render single ellipsoid
		_shader->setUniform("_modelMatrix", modelMatrix);
		_arrowGeometry->render(GL_TRIANGLE_STRIP);
    }

}
