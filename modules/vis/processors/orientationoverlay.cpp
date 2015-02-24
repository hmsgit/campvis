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

#include "orientationoverlay.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"
#include "cgt/texturereadertga.h"

#include "core/datastructures/cameradata.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/meshgeometry.h"
#include "core/datastructures/renderdata.h"

#include "core/tools/quadrenderer.h"

namespace campvis {
    
    const std::string OrientationOverlay::loggerCat_ = "CAMPVis.modules.vis.OrientationOverlay";

    OrientationOverlay::OrientationOverlay(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_camera("Camera", "Camera", "camera", DataNameProperty::READ)
        , p_sourceImageId("SourceImageId", "Source Image ID", "", DataNameProperty::READ)
        , p_targetImageId("TargetImageId", "Output Image", "OrientationOverlay", DataNameProperty::WRITE)
        , p_passThroughImageId("PassThroughImageId", "Passthrough Image ID (optional)", "", DataNameProperty::READ)
        , p_cubeSize("CubeSize", "Cube Size", .1f, .01f, .5f, .1f, 2)
        , p_cubePosition("CubePosition", "Cube Position", cgt::vec2(.85f, .15f), cgt::vec2(0.f), cgt::vec2(1.f), cgt::vec2(.1f), cgt::ivec2(2))
        , p_enableTexturing("EnableTexturing", "Enable Texturing", true)
        , p_flipMatrix("FlipMatrix", "Flip Matrix for Alignment (optional)", cgt::mat4::identity)
        , _cubeGeometry(nullptr)
        , _shader(nullptr)
        , _textures(nullptr)
    {
        addProperty(p_camera);
        addProperty(p_sourceImageId);
        addProperty(p_targetImageId);
        addProperty(p_passThroughImageId);

        addProperty(p_cubeSize, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_cubePosition);
        addProperty(p_enableTexturing);
        addProperty(p_flipMatrix);

        p_flipMatrix.setVisible(false);
    }

    OrientationOverlay::~OrientationOverlay() {

    }

    void OrientationOverlay::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/vis/glsl/orientationoverlay.frag", "");
        _passthroughShader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/vis/glsl/quadview.frag", "");

        createCube();

        std::vector<std::string> textureFileNames;
        textureFileNames.push_back(ShdrMgr.completePath("/modules/vis/textures/front.tga"));
        textureFileNames.push_back(ShdrMgr.completePath("/modules/vis/textures/back.tga"));
        textureFileNames.push_back(ShdrMgr.completePath("/modules/vis/textures/top.tga"));
        textureFileNames.push_back(ShdrMgr.completePath("/modules/vis/textures/bottom.tga"));
        textureFileNames.push_back(ShdrMgr.completePath("/modules/vis/textures/left.tga"));
        textureFileNames.push_back(ShdrMgr.completePath("/modules/vis/textures/right.tga"));

        cgt::TextureReaderTga tgaReader;
        _textures = tgaReader.loadTextureArray(textureFileNames, cgt::Texture::LINEAR);
    }

    void OrientationOverlay::deinit() {
        VisualizationProcessor::deinit();

        delete _cubeGeometry;
        ShdrMgr.dispose(_shader);
        ShdrMgr.dispose(_passthroughShader);

        delete _textures;
    }

    void OrientationOverlay::updateResult(DataContainer& data) {
        ScopedTypedData<CameraData> camera(data, p_camera.getValue());
        ScopedTypedData<ImageData> sourceImage(data, p_sourceImageId.getValue());
        ScopedTypedData<RenderData> passthroughImage(data, p_passThroughImageId.getValue());

        if (camera && sourceImage) {
            FramebufferActivationGuard fag(this);
            createAndAttachColorTexture();
            createAndAttachDepthTexture();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

            if (passthroughImage) {
                _passthroughShader->activate();

                cgt::TextureUnit colorUnit, depthUnit;
                passthroughImage->bind(_passthroughShader, colorUnit, depthUnit);
                QuadRdr.renderQuad11();

                _passthroughShader->deactivate();
            }

            glEnable(GL_DEPTH_TEST);
            _shader->activate();

            cgt::TextureUnit textureUnit;
            textureUnit.activate();
            _textures->bind();
            _shader->setUniform("_cubeTexture", textureUnit.getUnitNumber());
            _shader->setUniform("_enableTexturing", p_enableTexturing.getValue());

            _shader->setUniform("_projectionMatrix", cgt::mat4::createOrtho(-1,1,1,-1,-2,2));
            const cgt::mat4 viewMatrix = cgt::mat4::createTranslation(cgt::vec3(p_cubePosition.getValue().x * 2.f - 1.f, p_cubePosition.getValue().y * 2.f - 1.f, 0.f))
                                       * cgt::mat4::createScale(cgt::vec3(float(getEffectiveViewportSize().y) / float(getEffectiveViewportSize().x), 1.f, 1.f))
                                       * camera->getCamera().getViewMatrix().getRotationalPart();
            _shader->setUniform("_viewMatrix", viewMatrix);
            _shader->setUniform("_modelMatrix", p_flipMatrix.getValue());

            _cubeGeometry->render(GL_TRIANGLE_FAN);

            glDisable(GL_DEPTH_TEST);

            _shader->deactivate();
            cgt::TextureUnit::setZeroUnit();
            LGL_ERROR;

            data.addData(p_targetImageId.getValue(), new RenderData(_fbo));
        }
        else {
            LDEBUG("No suitable input images found.");
        }
    }

    void OrientationOverlay::createCube() {
        const float& cs = p_cubeSize.getValue();
        std::vector<cgt::vec3> vertices, texCoords;
        std::vector<FaceGeometry> faces;

        // bottom
        texCoords.push_back(cgt::vec3(0.f, 0.f, 3.f));
        vertices.push_back(cgt::vec3(-cs, -cs,  cs));
        texCoords.push_back(cgt::vec3(1.f, 0.f, 3.f));
        vertices.push_back(cgt::vec3( cs, -cs,  cs));
        texCoords.push_back(cgt::vec3(1.f, 1.f, 3.f));
        vertices.push_back(cgt::vec3( cs,  cs,  cs));
        texCoords.push_back(cgt::vec3(0.f, 1.f, 3.f));
        vertices.push_back(cgt::vec3(-cs,  cs,  cs));
        faces.push_back(FaceGeometry(vertices, texCoords, std::vector<cgt::vec4>(4, cgt::vec4(0.f, 0.f, 1.f, 1.f)), std::vector<cgt::vec3>(4, cgt::vec3(0.f, 0.f, 1.f))));
        vertices.clear();
        texCoords.clear();

        // top
        texCoords.push_back(cgt::vec3(0.f, 0.f, 2.f));
        vertices.push_back(cgt::vec3( cs, -cs, -cs));
        texCoords.push_back(cgt::vec3(1.f, 0.f, 2.f));
        vertices.push_back(cgt::vec3(-cs, -cs, -cs));
        texCoords.push_back(cgt::vec3(1.f, 1.f, 2.f));
        vertices.push_back(cgt::vec3(-cs,  cs, -cs));
        texCoords.push_back(cgt::vec3(0.f, 1.f, 2.f));
        vertices.push_back(cgt::vec3( cs,  cs, -cs));
        faces.push_back(FaceGeometry(vertices, texCoords, std::vector<cgt::vec4>(4, cgt::vec4(0.f, 0.f, 1.f, 1.f)), std::vector<cgt::vec3>(4, cgt::vec3(0.f, 0.f, 1.f))));
        vertices.clear();
        texCoords.clear();

        // front
        texCoords.push_back(cgt::vec3(0.f, 0.f, 0.f));
        vertices.push_back(cgt::vec3(-cs,  cs,  cs));
        texCoords.push_back(cgt::vec3(1.f, 0.f, 0.f));
        vertices.push_back(cgt::vec3( cs,  cs,  cs));
        texCoords.push_back(cgt::vec3(1.f, 1.f, 0.f));
        vertices.push_back(cgt::vec3( cs,  cs, -cs));
        texCoords.push_back(cgt::vec3(0.f, 1.f, 0.f));
        vertices.push_back(cgt::vec3(-cs,  cs, -cs));
        faces.push_back(FaceGeometry(vertices, texCoords, std::vector<cgt::vec4>(4, cgt::vec4(0.f, 1.f, 0.f, 1.f)), std::vector<cgt::vec3>(4, cgt::vec3(0.f, 0.f, 1.f))));
        vertices.clear();
        texCoords.clear();

        // back
        texCoords.push_back(cgt::vec3(0.f, 0.f, 1.f));
        vertices.push_back(cgt::vec3( cs, -cs,  cs));
        texCoords.push_back(cgt::vec3(1.f, 0.f, 1.f));
        vertices.push_back(cgt::vec3(-cs, -cs,  cs));
        texCoords.push_back(cgt::vec3(1.f, 1.f, 1.f));
        vertices.push_back(cgt::vec3(-cs, -cs, -cs));
        texCoords.push_back(cgt::vec3(0.f, 1.f, 1.f));
        vertices.push_back(cgt::vec3( cs, -cs, -cs));
        faces.push_back(FaceGeometry(vertices, texCoords, std::vector<cgt::vec4>(4, cgt::vec4(0.f, 1.f, 0.f, 1.f)), std::vector<cgt::vec3>(4, cgt::vec3(0.f, 0.f, 1.f))));
        vertices.clear();
        texCoords.clear();

        // right
        texCoords.push_back(cgt::vec3(0.f, 0.f, 5.f));
        vertices.push_back(cgt::vec3(-cs, -cs,  cs));
        texCoords.push_back(cgt::vec3(1.f, 0.f, 5.f));
        vertices.push_back(cgt::vec3(-cs,  cs,  cs));
        texCoords.push_back(cgt::vec3(1.f, 1.f, 5.f));
        vertices.push_back(cgt::vec3(-cs,  cs, -cs));
        texCoords.push_back(cgt::vec3(0.f, 1.f, 5.f));
        vertices.push_back(cgt::vec3(-cs, -cs, -cs));
        faces.push_back(FaceGeometry(vertices, texCoords, std::vector<cgt::vec4>(4, cgt::vec4(1.f, 0.f, 0.f, 1.f)), std::vector<cgt::vec3>(4, cgt::vec3(0.f, 0.f, 1.f))));
        vertices.clear();
        texCoords.clear();

        // left
        texCoords.push_back(cgt::vec3(0.f, 0.f, 4.f));
        vertices.push_back(cgt::vec3( cs,  cs,  cs));
        texCoords.push_back(cgt::vec3(1.f, 0.f, 4.f));
        vertices.push_back(cgt::vec3( cs, -cs,  cs));
        texCoords.push_back(cgt::vec3(1.f, 1.f, 4.f));
        vertices.push_back(cgt::vec3( cs, -cs, -cs));
        texCoords.push_back(cgt::vec3(0.f, 1.f, 4.f));
        vertices.push_back(cgt::vec3( cs,  cs, -cs));
        faces.push_back(FaceGeometry(vertices, texCoords, std::vector<cgt::vec4>(4, cgt::vec4(1.f, 0.f, 0.f, 1.f)), std::vector<cgt::vec3>(4, cgt::vec3(0.f, 0.f, 1.f))));
        vertices.clear();
        texCoords.clear();

        delete _cubeGeometry;
        _cubeGeometry = new MeshGeometry(faces);
    }

    void OrientationOverlay::updateProperties(DataContainer& dataContainer) {
        createCube();
    }

}

