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

#include "eepgenerator.h"

#include "cgt/glmath.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"
#include "core/datastructures/renderdata.h"
#include "core/datastructures/meshgeometry.h"
#include "core/pipeline/processordecoratormasking.h"

namespace campvis {
    const std::string EEPGenerator::loggerCat_ = "CAMPVis.modules.vis.EEPGenerator";

    EEPGenerator::EEPGenerator(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_sourceImageID("sourceImageID", "Input Image", "", DataNameProperty::READ)
        , p_geometryID("geometryID", "Input Proxy Geometry ID", "proxygeometry", DataNameProperty::READ)
        , p_geometryImageId("GeometryImageId", "Rendered Geometry to Integrate (optional)", "", DataNameProperty::READ)
        , p_entryImageID("entryImageID", "Output Entry Points Image", "eep.entry", DataNameProperty::WRITE)
        , p_exitImageID("exitImageID", "Output Exit Points Image", "eep.exit", DataNameProperty::WRITE)
        , p_camera("camera", "Camera")
        , p_enableMirror("enableMirror", "Enable Virtual Mirror Feature", false)
        , p_mirrorID("mirrorID", "Input Mirror ID", "", DataNameProperty::READ)
        , _shader(0)
    {
        addDecorator(new ProcessorDecoratorMasking());

        addProperty(p_sourceImageID);
        addProperty(p_geometryID);
        addProperty(p_geometryImageId);
        addProperty(p_entryImageID);
        addProperty(p_exitImageID);
        addProperty(p_camera);

        addProperty(p_enableMirror, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_mirrorID);
        p_mirrorID.setVisible(false);

        decoratePropertyCollection(this);
    }

    EEPGenerator::~EEPGenerator() {

    }

    void EEPGenerator::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/vis/glsl/eepgenerator.frag", generateHeader());
        if (_shader != 0) {
            _shader->setAttributeLocation(0, "in_Position");
            _shader->setAttributeLocation(1, "in_TexCoord");
        }
    }

    void EEPGenerator::deinit() {
        ShdrMgr.dispose(_shader);
        _shader = 0;
        VisualizationProcessor::deinit();
    }

    void EEPGenerator::updateResult(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_sourceImageID.getValue());
        ScopedTypedData<MeshGeometry> proxyGeometry(data, p_geometryID.getValue());

        if (img != 0 && proxyGeometry != 0 && _shader != 0) {
            if (img->getDimensionality() == 3) {
                ScopedTypedData<RenderData> geometryImage(data, p_geometryImageId.getValue());

                cgt::Bounds textureBounds(cgt::vec3(0.f), cgt::vec3(1.f));

                // clip proxy geometry against near-plane to support camera in volume
                // FIXME:   In some cases, the near plane is not rendered correctly...
                float nearPlaneDistToOrigin = cgt::dot(p_camera.getValue().getPosition(), -p_camera.getValue().getLook()) - p_camera.getValue().getNearDist() - .002f;
                MeshGeometry clipped = proxyGeometry->clipAgainstPlane(nearPlaneDistToOrigin, -p_camera.getValue().getLook(), true, 0.02f);

                // start render setup
                _shader->activate();

                // setup virtual mirror if necessary
                cgt::mat4 mirrorMatrix = cgt::mat4::identity;
                if (p_enableMirror.getValue()) {
                    ScopedTypedData<FaceGeometry> mirrorGeometry(data, p_mirrorID.getValue());
                    if (mirrorGeometry && mirrorGeometry->size() > 0) {
                        const cgt::vec3& p = mirrorGeometry->getVertices()[0];
                        cgt::vec3 n = cgt::normalize(cgt::cross(mirrorGeometry->getVertices()[1] - mirrorGeometry->getVertices()[0], mirrorGeometry->getVertices()[2] - mirrorGeometry->getVertices()[0]));
                        float k = cgt::dot(p, n);

                        // mirror matrix sponsored by:
                        // Jiang 
                        mirrorMatrix = cgt::transpose(cgt::mat4(
                            1 - 2*n.x*n.x, -2*n.y*n.x   , -2*n.z*n.x   , 0, 
                            -2*n.x*n.y   , 1 - 2*n.y*n.y, -2*n.z*n.y   , 0, 
                            -2*n.x*n.z   , -2*n.y*n.z   , 1 - 2*n.z*n.z, 0, 
                            2*n.x*k      , 2*n.y*k      , 2*n.z*k      , 1));
                    }
                    else {
                        LERROR("No suitable virtual mirror geometry found.");
                    }
                }
                
                const cgt::Camera& cam = p_camera.getValue();
                cgt::TextureUnit geometryDepthUnit, entryDepthUnit;

                _shader->setIgnoreUniformLocationError(true);
                _shader->setUniform("_viewportSizeRCP", 1.f / cgt::vec2(getEffectiveViewportSize()));
                _shader->setUniform("_modelMatrix", mirrorMatrix);
                _shader->setUniform("_projectionMatrix", cam.getProjectionMatrix());
                _shader->setUniform("_viewMatrix", cam.getViewMatrix());

                if (geometryImage != 0) {
                    geometryImage->bindDepthTexture(_shader, geometryDepthUnit, "_geometryDepthTexture", "_geometryDepthTexParams");

                    _shader->setUniform("_integrateGeometry", true);
                    _shader->setUniform("_near", cam.getNearDist());
                    _shader->setUniform("_far", cam.getFarDist());

                    cgt::mat4 inverseView = cgt::mat4::identity;
                    if (cam.getViewMatrix().invert(inverseView))
                        _shader->setUniform("_inverseViewMatrix", inverseView);

                    cgt::mat4 inverseProjection = cgt::mat4::identity;
                    if (cam.getProjectionMatrix().invert(inverseProjection))
                        _shader->setUniform("_inverseProjectionMatrix", inverseProjection);

                    _shader->setUniform("_volumeWorldToTexture", img->getParent()->getMappingInformation().getWorldToTextureMatrix());
                }
                else {
                    _shader->setUniform("_integrateGeometry", false);
                }

                _shader->setIgnoreUniformLocationError(false);

                FramebufferActivationGuard fag(this);
                decorateRenderProlog(data, _shader);
                glEnable(GL_CULL_FACE);
                glEnable(GL_DEPTH_TEST);

                // create entry points texture
                createAndAttachTexture(GL_RGBA16);
                createAndAttachDepthTexture();
                _shader->setUniform("_isEntrypoint", true);

                glDepthFunc(GL_LESS);
                glClearDepth(1.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glCullFace(p_enableMirror.getValue() ? GL_FRONT : GL_BACK);
                clipped.render(GL_POLYGON);

                RenderData* entrypoints = new RenderData(_fbo);
                _fbo->detachAll();

                // create exit points texture
                createAndAttachTexture(GL_RGBA16);
                createAndAttachDepthTexture();
                _shader->setUniform("_isEntrypoint", false);

                if (geometryImage != 0) {
                    entrypoints->bindDepthTexture(_shader, entryDepthUnit, "_entryDepthTexture", "_entryDepthTexParams");
                }

                glDepthFunc(GL_GREATER);
                glClearDepth(0.0f);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                glCullFace(p_enableMirror.getValue() ? GL_BACK : GL_FRONT);
                clipped.render(GL_POLYGON);

                RenderData* exitpoints = new RenderData(_fbo);
                decorateRenderEpilog(_shader);
                _shader->deactivate();

                glDepthFunc(GL_LESS);
                glClearDepth(1.0f);
                glCullFace(GL_BACK);
                glDisable(GL_CULL_FACE);
                glDisable(GL_DEPTH_TEST);

                LGL_ERROR;

                data.addData(p_entryImageID.getValue(), entrypoints);
                data.addData(p_exitImageID.getValue(), exitpoints);
            }
            else {
                LERROR("Input image must have dimensionality of 3.");
            }
        }
        else {
            LDEBUG("No suitable input image or proxy geometry found.");
        }
    }

    std::string EEPGenerator::generateHeader() const {
        return getDecoratedHeader();
    }

    void EEPGenerator::updateProperties(DataContainer& dataContainer) {
        p_mirrorID.setVisible(p_enableMirror.getValue());
    }

    void EEPGenerator::updateShader() {
        _shader->setHeaders(generateHeader());
        _shader->rebuild();
    }

}
