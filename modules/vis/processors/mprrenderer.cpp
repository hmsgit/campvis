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

#include "mprrenderer.h"
#include "tgt/bounds.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"

#include "core/datastructures/meshgeometry.h"
#include "core/datastructures/facegeometry.h"
#include "core/datastructures/geometrydatafactory.h"

#include "core/classification/simpletransferfunction.h"

#include "core/tools/quadrenderer.h"

#include "core/datastructures/transformdata.h"

namespace campvis {
    const std::string MprRenderer::loggerCat_ = "CAMPVis.modules.vis.MprRenderer";

    MprRenderer::MprRenderer(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_sourceImageID("sourceImageID", "Input Image", "", DataNameProperty::READ)
        , p_targetImageID("targetImageID", "Output Image", "", DataNameProperty::WRITE)
        , p_camera("Camera", "Camera")
		, p_transformationID("transformationID", "Input Transformation", "", DataNameProperty::READ)
        , p_planeNormal("PlaneNormal", "Clipping Plane Normal", tgt::vec3(0.f, 0.f, 1.f), tgt::vec3(-1.f), tgt::vec3(1.f), tgt::vec3(.1f), tgt::ivec3(2))
        , p_planeDistance("PlaneDistance", "Clipping Plane Distance", 0.f, -1000.f, 1000.f, 1.f, 1)
        , p_planeSize("PlaneSize", "Clipping Plane Size", 100.f, 0.f, 1000.f, 1.f, 1)
        , p_use2DProjection("Use3dRendering", "Use 3D Rendering instead of 2D", true)
        , p_relativeToImageCenter("RelativeToImageCenter", "Construct Plane Relative to Image Center", true)
        , p_transferFunction("transferFunction", "Transfer Function", new SimpleTransferFunction(256))
        , _shader(nullptr)
    {
        addProperty(p_sourceImageID, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_targetImageID);
        addProperty(p_camera);
		addProperty(p_transformationID);
        addProperty(p_planeNormal);
        addProperty(p_planeDistance);
        addProperty(p_planeSize);
        addProperty(p_use2DProjection, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_relativeToImageCenter);
        addProperty(p_transferFunction);
    }

    MprRenderer::~MprRenderer() {

    }

    void MprRenderer::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/vis/glsl/mprrenderer.frag", "");
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void MprRenderer::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void MprRenderer::updateResult(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_sourceImageID.getValue());

        if (img != 0) {
            if (img->getDimensionality() == 3) {
                const tgt::Camera& cam = p_camera.getValue();

                // Construct the clipping plane in world coordinates
                tgt::vec3 n = tgt::normalize(p_planeNormal.getValue());
                tgt::vec3 temp(1.0, 0.0, 0.0);
                if (abs(tgt::dot(temp, n) > 0.9))
                    temp = tgt::vec3(0.0, 1.0, 0.0);

				tgt::vec3 inPlaneA;
				tgt::vec3 inPlaneB;
				tgt::vec3 base;

				{
					ScopedTypedData<TransformData> td(data, p_transformationID.getValue());
					if (td) 
                    {
						// convention for now: the slice in xy plane centered at (0,0,0) is sampled
						// coordinate system supplied by the transformation
						tgt::mat4 t = td->getTransform();
						
						base = t * tgt::vec3(0, 0, 0);
                        inPlaneA = tgt::normalize((t * tgt::vec4(1, 0, 0, 0)).xyz()) * 0.5f * p_planeSize.getValue();
                        inPlaneB = tgt::normalize((t * tgt::vec4(0, 1, 0, 0)).xyz()) * 0.5f * p_planeSize.getValue();
					}
					else {
						inPlaneA = tgt::normalize(tgt::cross(n, temp)) * 0.5f * p_planeSize.getValue();
						inPlaneB = tgt::normalize(tgt::cross(n, inPlaneA)) * 0.5f * p_planeSize.getValue();
						base = (n * -p_planeDistance.getValue());

						// move to image center if wanted
						if (p_relativeToImageCenter.getValue())
							base += img->getParent()->getWorldBounds().center();
					}
				}

				// construct the four texCoords
				std::vector<tgt::vec3> texCoords;

				texCoords.push_back(base + inPlaneA + inPlaneB);
				texCoords.push_back(base - inPlaneA + inPlaneB);
				texCoords.push_back(base - inPlaneA - inPlaneB);
				texCoords.push_back(base + inPlaneA - inPlaneB);

                FaceGeometry slice(texCoords, texCoords);

                // perform the rendering
                glEnable(GL_DEPTH_TEST);
                _shader->activate();
                tgt::Shader::IgnoreUniformLocationErrorGuard guard(_shader);

                if (p_use2DProjection.getValue()) {
                    // generate a camera position that simulates 2D rendering
                    // this way it is easier to achieve the correct aspect ratio in all cases
                    tgt::vec3 camPosition = base - p_planeSize.getValue() * n;
                    float ratio = static_cast<float>(getEffectiveViewportSize().x) / getEffectiveViewportSize().y;

                    // experimentally discovered: 
                    // if the camera distance is half as big as the plane size, a field of view of
                    // 54 allows to see everything
                    float fovy = 54.f;

                    tgt::Camera c(camPosition, base, inPlaneA, fovy, ratio, 0.1f, 10000.f);
                    _shader->setUniform("_projectionMatrix", c.getProjectionMatrix());
                    _shader->setUniform("_viewMatrix", c.getViewMatrix());
                }
                else {
                    _shader->setUniform("_projectionMatrix", cam.getProjectionMatrix());
                    _shader->setUniform("_viewMatrix", cam.getViewMatrix());
                }

                tgt::TextureUnit inputUnit, tfUnit;
                img->bind(_shader, inputUnit);
                p_transferFunction.getTF()->bind(_shader, tfUnit);

                FramebufferActivationGuard fag(this);
                createAndAttachColorTexture();
                createAndAttachDepthTexture();
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                slice.render(GL_POLYGON);

                _shader->deactivate();
                tgt::TextureUnit::setZeroUnit();
                glDisable(GL_DEPTH_TEST);

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

    void MprRenderer::updateProperties(DataContainer& dc) {
        ScopedTypedData<ImageData> img(dc, p_sourceImageID.getValue());

        if (img != 0) {
            p_transferFunction.setVisible(img->getNumChannels() == 1);
        }

        p_transferFunction.setImageHandle(img.getDataHandle());
        p_camera.setVisible(!p_use2DProjection.getValue());
                        
        validate(AbstractProcessor::INVALID_PROPERTIES);
    }

}
