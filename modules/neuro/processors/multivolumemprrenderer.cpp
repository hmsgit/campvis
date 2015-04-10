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

#include "multivolumemprrenderer.h"

#include "cgt/glmath.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"

#include "core/classification/simpletransferfunction.h"
#include "core/datastructures/cameradata.h"
#include "core/datastructures/geometrydatafactory.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/imageseries.h"
#include "core/datastructures/lightsourcedata.h"
#include "core/datastructures/renderdata.h"
#include "core/datastructures/meshgeometry.h"
#include "core/pipeline/processordecoratorgradient.h"
#include "core/tools/quadrenderer.h"

namespace campvis {
namespace neuro {

    const std::string MultiVolumeMprRenderer::loggerCat_ = "CAMPVis.modules.vis.MultiVolumeMprRenderer";

    MultiVolumeMprRenderer::MultiVolumeMprRenderer(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_sourceImage1("SourceImage1", "First Input Image", "", DataNameProperty::READ)
        , p_sourceImage2("SourceImage2", "Second Input Image", "", DataNameProperty::READ)
        , p_sourceImage3("SourceImage3", "Third Input Image", "", DataNameProperty::READ)
        , p_camera("Camera", "Camera ID", "camera", DataNameProperty::READ)
        , p_outputImageId("OutputImageId", "Output Image ID", "MultiVolumeMprRenderer.output", DataNameProperty::WRITE)
        , p_transferFunction1("TransferFunction1", "Transfer Function for First image", new SimpleTransferFunction(256))
        , p_transferFunction2("TransferFunction2", "Transfer Function for Second image", new SimpleTransferFunction(256))
        , p_transferFunction3("TransferFunction3", "Transfer Function for Third image", new SimpleTransferFunction(256))
        , p_planeNormal("PlaneNormal", "Clipping Plane Normal", cgt::vec3(0.f, 0.f, 1.f), cgt::vec3(-1.f), cgt::vec3(1.f), cgt::vec3(.1f), cgt::ivec3(2))
        , p_planeDistance("PlaneDistance", "Clipping Plane Distance", 0.f, -1000.f, 1000.f, .5f, 1)
        , p_planeSize("PlaneSize", "Clipping Plane Size", 100.f, 0.f, 1000.f, 1.f, 1)
        , p_use2DProjection("Use3dRendering", "Use 3D Rendering instead of 2D", true)
        , p_relativeToImageCenter("RelativeToImageCenter", "Construct Plane Relative to Image Center", true)
        , p_showWireframe("ShowWireframe", "Show Wireframe", true)
        , p_lineWidth("LineWidth", "Line Width", 1.f, .1f, 10.f)
        , p_transparency("Transparency", "Minimum Transparency", 0.5f, 0.f, 1.f)
        , _shader(nullptr)
    {
        addProperty(p_sourceImage1, INVALID_PROPERTIES | INVALID_RESULT);
        addProperty(p_sourceImage2, INVALID_PROPERTIES | INVALID_RESULT);
        addProperty(p_sourceImage3, INVALID_PROPERTIES | INVALID_RESULT);
        addProperty(p_camera);
        addProperty(p_outputImageId);

        addProperty(p_transferFunction1);
        addProperty(p_transferFunction2);
        addProperty(p_transferFunction3);

        addProperty(p_planeNormal);
        addProperty(p_planeDistance);
        addProperty(p_planeSize);
        addProperty(p_use2DProjection, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_relativeToImageCenter);

        addProperty(p_showWireframe, INVALID_RESULT | INVALID_SHADER);
        addProperty(p_lineWidth);
        addProperty(p_transparency);
    }

    MultiVolumeMprRenderer::~MultiVolumeMprRenderer() {

    }

    void MultiVolumeMprRenderer::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.load("modules/vis/glsl/geometryrenderer.vert", "modules/vis/glsl/geometryrenderer.geom", "modules/neuro/glsl/multivolumemprrenderer.frag", generateHeader());
    }

    void MultiVolumeMprRenderer::deinit() {
        ShdrMgr.dispose(_shader);
        _shader = nullptr;

        VisualizationProcessor::deinit();
    }

    void MultiVolumeMprRenderer::updateResult(DataContainer& dataContainer) {
        ImageRepresentationGL::ScopedRepresentation image1(dataContainer, p_sourceImage1.getValue());
        ImageRepresentationGL::ScopedRepresentation image2(dataContainer, p_sourceImage2.getValue());
        ImageRepresentationGL::ScopedRepresentation image3(dataContainer, p_sourceImage3.getValue());
        ScopedTypedData<CameraData> camera(dataContainer, p_camera.getValue());


        if (image1 && image2 && image3 && camera) {
            if (p_use2DProjection.getValue() || camera != nullptr) {
                // Construct the clipping plane in world coordinates
                cgt::vec3 n = cgt::normalize(p_planeNormal.getValue());
                cgt::vec3 temp(1.0, 0.0, 0.0);
                if (abs(cgt::dot(temp, n)) > 0.9)
                    temp = cgt::vec3(0.0, 1.0, 0.0);

                cgt::vec3 inPlaneA = cgt::normalize(cgt::cross(n, temp)) * 0.5f * p_planeSize.getValue();
                cgt::vec3 inPlaneB = cgt::normalize(cgt::cross(n, inPlaneA)) * 0.5f * p_planeSize.getValue();
                cgt::vec3 base = (n * -p_planeDistance.getValue());

                // move to image center if wanted
                if (p_relativeToImageCenter.getValue()) {
                    cgt::Bounds b;
                    if (image1)
                        b.addVolume(image1->getParent()->getWorldBounds());
                    if (image2)
                        b.addVolume(image2->getParent()->getWorldBounds());
                    if (image3)
                        b.addVolume(image3->getParent()->getWorldBounds());
                    base += b.center();
                }

                // construct the four texCoords
                std::vector<cgt::vec3> texCoords;
                texCoords.push_back(base + inPlaneA + inPlaneB);
                texCoords.push_back(base - inPlaneA + inPlaneB);
                texCoords.push_back(base - inPlaneA - inPlaneB);
                texCoords.push_back(base + inPlaneA - inPlaneB);
                FaceGeometry slice(texCoords, texCoords);

                // perform the rendering
                glEnable(GL_DEPTH_TEST);
                _shader->activate();
                if (p_showWireframe.getValue()) {
                    // calculate viewport matrix for NDC -> viewport conversion
                    cgt::vec2 halfViewport = cgt::vec2(getEffectiveViewportSize()) / 2.f;
                    cgt::mat4 viewportMatrix = cgt::mat4::createTranslation(cgt::vec3(halfViewport, 0.f)) * cgt::mat4::createScale(cgt::vec3(halfViewport, 1.f));
                    _shader->setUniform("_viewportMatrix", viewportMatrix);

                    _shader->setUniform("_lineWidth", p_lineWidth.getValue());
                    _shader->setUniform("_transparency", p_transparency.getValue());
                }

                if (p_use2DProjection.getValue()) {
                    // generate a camera position that simulates 2D rendering
                    // this way it is easier to achieve the correct aspect ratio in all cases
                    cgt::vec3 camPosition = base - p_planeSize.getValue() * n;
                    float ratio = static_cast<float>(getEffectiveViewportSize().x) / getEffectiveViewportSize().y;

                    // experimentally discovered: 
                    // if the camera distance is half as big as the plane size, a field of view of
                    // 54 allows to see everything
                    float fovy = 54.f;

                    cgt::Camera c(camPosition, base, inPlaneA, fovy, ratio, 0.1f, 10000.f);
                    _shader->setUniform("_projectionMatrix", c.getProjectionMatrix());
                    _shader->setUniform("_viewMatrix", c.getViewMatrix());
                }
                else {
                    _shader->setUniform("_projectionMatrix", camera->getCamera().getProjectionMatrix());
                    _shader->setUniform("_viewMatrix", camera->getCamera().getViewMatrix());
                }

                cgt::TextureUnit volumeUnit1, volumeUnit2, volumeUnit3, tf1Unit, tf2Unit, tf3Unit;
                image1->bind(_shader, volumeUnit1, "_volume1", "_volumeParams1");
                image2->bind(_shader, volumeUnit2, "_volume2", "_volumeParams2");
                image3->bind(_shader, volumeUnit3, "_volume3", "_volumeParams3");
                p_transferFunction1.getTF()->bind(_shader, tf1Unit, "_transferFunction1", "_transferFunctionParams1");
                p_transferFunction2.getTF()->bind(_shader, tf2Unit, "_transferFunction2", "_transferFunctionParams2");
                p_transferFunction3.getTF()->bind(_shader, tf3Unit, "_transferFunction3", "_transferFunctionParams3");

                FramebufferActivationGuard fag(this);
                createAndAttachTexture(GL_RGBA8);
                createAndAttachTexture(GL_RGBA32F);
                createAndAttachDepthTexture();

                static const GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 };
                glDrawBuffers(2, buffers);
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                slice.render(GL_TRIANGLE_FAN);

                glDrawBuffers(1, buffers);
                _shader->deactivate();
                cgt::TextureUnit::setZeroUnit();
                glDisable(GL_DEPTH_TEST);

                dataContainer.addData(p_outputImageId.getValue(), new RenderData(_fbo));
            }
        }
        else {
            LDEBUG("No suitable input data found!");
        }
    }

    std::string MultiVolumeMprRenderer::generateHeader() const {
        std::string toReturn = "#define HAS_GEOMETRY_SHADER\n";

        if (p_showWireframe.getValue())
            toReturn += "#define WIREFRAME_RENDERING\n";

        return toReturn;
    }

    void MultiVolumeMprRenderer::updateProperties(DataContainer& dataContainer) {
        ImageRepresentationGL::ScopedRepresentation image1(dataContainer, p_sourceImage1.getValue());
        ImageRepresentationGL::ScopedRepresentation image2(dataContainer, p_sourceImage2.getValue());
        ImageRepresentationGL::ScopedRepresentation image3(dataContainer, p_sourceImage3.getValue());

        if (image1)
            p_transferFunction1.setImageHandle(image1.getDataHandle());
        else
            p_transferFunction1.setImageHandle(DataHandle(nullptr));

        if (image2)
            p_transferFunction2.setImageHandle(image2.getDataHandle());
        else
            p_transferFunction2.setImageHandle(DataHandle(nullptr));

        if (image3)
            p_transferFunction3.setImageHandle(image3.getDataHandle());
        else
            p_transferFunction3.setImageHandle(DataHandle(nullptr));
    }

    void MultiVolumeMprRenderer::updateShader() {
        _shader->setHeaders(generateHeader());
        _shader->rebuild();
    }

}
}
