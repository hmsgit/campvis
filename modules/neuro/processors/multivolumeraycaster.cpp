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

#include "multivolumeraycaster.h"

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

    const std::string MultiVolumeRaycaster::loggerCat_ = "CAMPVis.modules.vis.MultiVolumeRaycaster";

    MultiVolumeRaycaster::MultiVolumeRaycaster(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_sourceImagesId("SourceImagesId", "Input Image(s)", "", DataNameProperty::READ)
        , p_geometryImageId("GeometryImageId", "Rendered Geometry to Integrate (optional)", "", DataNameProperty::READ)
        , p_camera("Camera", "Camera ID", "camera", DataNameProperty::READ)
        , p_lightId("LightId", "Input Light Source", "lightsource", DataNameProperty::READ)
        , p_outputImageId("OutputImageId", "Output Image ID", "MultiVolumeRaycaster.output", DataNameProperty::WRITE)
        , p_transferFunction1("TransferFunction1", "Transfer Function for First image", new SimpleTransferFunction(256))
        , p_transferFunction2("TransferFunction2", "Transfer Function for Second image", new SimpleTransferFunction(256))
        , p_transferFunction3("TransferFunction3", "Transfer Function for Third image", new SimpleTransferFunction(256))
        , p_jitterStepSizeMultiplier("jitterStepSizeMultiplier", "Jitter Step Size Multiplier", 1.f, 0.f, 1.f)
        , p_samplingRate("SamplingRate", "Sampling Rate", 2.f, 0.1f, 10.f, 0.1f)
        , _eepShader(nullptr)
    {
        addDecorator(new ProcessorDecoratorGradient());

        addProperty(p_sourceImagesId, INVALID_PROPERTIES | INVALID_RESULT);
        addProperty(p_geometryImageId);
        addProperty(p_camera);
        addProperty(p_lightId);
        addProperty(p_outputImageId);

        addProperty(p_transferFunction1);
        addProperty(p_transferFunction2);
        addProperty(p_transferFunction3);
        addProperty(p_jitterStepSizeMultiplier);
        addProperty(p_samplingRate);

        decoratePropertyCollection(this);
    }

    MultiVolumeRaycaster::~MultiVolumeRaycaster() {

    }

    void MultiVolumeRaycaster::init() {
        VisualizationProcessor::init();

        _eepShader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/neuro/glsl/multivolumeraycaster_eep.frag", generateHeader());
        if (_eepShader != nullptr) {
            _eepShader->setAttributeLocation(0, "in_Position");
            _eepShader->setAttributeLocation(1, "in_TexCoord");
        }

        _rcShader = ShdrMgr.loadWithCustomGlslVersion("core/glsl/passthrough.vert", "", "modules/neuro/glsl/multivolumeraycaster_rc.frag", generateHeader(), "400");
        if (_rcShader != nullptr) {
            _rcShader->setAttributeLocation(0, "in_Position");
            _rcShader->setAttributeLocation(1, "in_TexCoord");
        }
    }

    void MultiVolumeRaycaster::deinit() {
        ShdrMgr.dispose(_eepShader);
        _eepShader = 0;
        VisualizationProcessor::deinit();
    }

    void MultiVolumeRaycaster::updateResult(DataContainer& dataContainer) {
        ImageRepresentationGL::ScopedRepresentation singleImage(dataContainer, p_sourceImagesId.getValue(), true);
        ScopedTypedData<ImageSeries> imageSeries(dataContainer, p_sourceImagesId.getValue(), true);
        ScopedTypedData<CameraData> camera(dataContainer, p_camera.getValue());
        ScopedTypedData<RenderData> geometryImage(dataContainer, p_geometryImageId.getValue(), true);
        ScopedTypedData<LightSourceData> light(dataContainer, p_lightId.getValue());

        std::vector<const ImageRepresentationGL*> images;
        if (singleImage != nullptr)
            images.push_back(singleImage);
        else if (imageSeries != nullptr) {
            for (size_t i = 0; i < imageSeries->getNumImages(); ++i) {
                images.push_back(static_cast<const ImageData*>(imageSeries->getImage(i).getData())->getRepresentation<ImageRepresentationGL>());
                cgtAssert(images.back() != nullptr, "We have a nullptr in our image list, this is WRONG! Did a conversion fail?");
            }
        }

        if (images.size() >= 3 && camera != nullptr) {
            auto eepp = computeEntryExitPoints(images, camera, geometryImage);
            dataContainer.addData(p_outputImageId.getValue() + ".entrypoints", eepp.first);
            dataContainer.addData(p_outputImageId.getValue() + ".exitpoints", eepp.second);

            auto rc = performRaycasting(dataContainer, images, camera, eepp.first, eepp.second, light);
            dataContainer.addData(p_outputImageId.getValue(), rc);
        }
        else {
            LDEBUG("No suitable input data found!");
        }
    }

    std::string MultiVolumeRaycaster::generateHeader() const {
        return getDecoratedHeader();
    }

    void MultiVolumeRaycaster::updateProperties(DataContainer& dataContainer) {
        ScopedTypedData<ImageData> img(dataContainer, p_sourceImagesId.getValue(), true);
        ScopedTypedData<ImageSeries> is(dataContainer, p_sourceImagesId.getValue(), true);

        if (img != nullptr) {
            p_transferFunction1.setImageHandle(img.getDataHandle());
        }
        else if (is != nullptr && is->getNumImages() == 3) {
            p_transferFunction1.setImageHandle(is->getImage(0));
            p_transferFunction2.setImageHandle(is->getImage(1));
            p_transferFunction3.setImageHandle(is->getImage(2));
        }
        else {
            p_transferFunction1.setImageHandle(DataHandle(nullptr));
            p_transferFunction2.setImageHandle(DataHandle(nullptr));
            p_transferFunction3.setImageHandle(DataHandle(nullptr));
        }
    }

    void MultiVolumeRaycaster::updateShader() {
        _eepShader->setHeaders(generateHeader());
        _eepShader->rebuild();
    }

    std::pair<RenderData*, RenderData*> MultiVolumeRaycaster::computeEntryExitPoints(const std::vector<const ImageRepresentationGL*>& images, const CameraData* camera, const RenderData* geometryImage) {
        cgtAssert(_eepShader != nullptr, "EEP Shader must not be 0.");
        const cgt::Camera& cam = camera->getCamera();

        // construct bounding box for all images
        cgt::Bounds b;
        for (size_t i = 0; i < images.size(); ++i) {
            b.addVolume(images[i]->getParent()->getWorldBounds());
        }

        // create proxy geometry
        std::unique_ptr<MeshGeometry> cube(GeometryDataFactory::createCube(b, b));
        // clip proxy geometry against near-plane to support camera in volume
        float nearPlaneDistToOrigin = cgt::dot(cam.getPosition(), -cam.getLook()) - cam.getNearDist() - .002f;
        MeshGeometry clipped = cube->clipAgainstPlane(nearPlaneDistToOrigin, -cam.getLook(), true, 0.02f);

        _eepShader->activate();

        cgt::TextureUnit geometryDepthUnit, entryDepthUnit;
        _eepShader->setUniform("_viewportSizeRCP", 1.f / cgt::vec2(getEffectiveViewportSize()));
        _eepShader->setUniform("_projectionMatrix", cam.getProjectionMatrix());
        _eepShader->setUniform("_viewMatrix", cam.getViewMatrix());

        if (geometryImage != nullptr) {
            geometryImage->bindDepthTexture(_eepShader, geometryDepthUnit, "_geometryDepthTexture", "_geometryDepthTexParams");

            _eepShader->setUniform("_integrateGeometry", true);
            _eepShader->setUniform("_near", cam.getNearDist());
            _eepShader->setUniform("_far", cam.getFarDist());

            cgt::mat4 inverseView = cgt::mat4::identity;
            if (cam.getViewMatrix().invert(inverseView))
                _eepShader->setUniform("_inverseViewMatrix", inverseView);

            cgt::mat4 inverseProjection = cgt::mat4::identity;
            if (cam.getProjectionMatrix().invert(inverseProjection))
                _eepShader->setUniform("_inverseProjectionMatrix", inverseProjection);
        }
        else {
            _eepShader->setUniform("_integrateGeometry", false);
        }

        glEnable(GL_CULL_FACE);
        glEnable(GL_DEPTH_TEST);

        // create entry points texture
        FramebufferActivationGuard fag(this);
        createAndAttachTexture(GL_RGBA32F);
        createAndAttachDepthTexture();
        _eepShader->setUniform("_isEntrypoint", true);

        glDepthFunc(GL_LESS);
        glClearDepth(1.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_BACK);
        clipped.render(GL_TRIANGLE_FAN);

        RenderData* entrypoints = new RenderData(_fbo);
        _fbo->detachAll();

        // create exit points texture
        createAndAttachTexture(GL_RGBA32F);
        createAndAttachDepthTexture();
        _eepShader->setUniform("_isEntrypoint", false);

        if (geometryImage != nullptr) {
            entrypoints->bindDepthTexture(_eepShader, entryDepthUnit, "_entryDepthTexture", "_entryDepthTexParams");
        }

        glDepthFunc(GL_GREATER);
        glClearDepth(0.0f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glCullFace(GL_FRONT);
        clipped.render(GL_TRIANGLE_FAN);

        RenderData* exitpoints = new RenderData(_fbo);
        decorateRenderEpilog(_eepShader);
        _eepShader->deactivate();

        glDepthFunc(GL_LESS);
        glClearDepth(1.0f);
        glCullFace(GL_BACK);
        glDisable(GL_CULL_FACE);
        glDisable(GL_DEPTH_TEST);

        _eepShader->deactivate();
        LGL_ERROR;

        return std::make_pair(entrypoints, exitpoints);
    }

    RenderData* MultiVolumeRaycaster::performRaycasting(DataContainer& dataContainer, const std::vector<const ImageRepresentationGL*>& images, const CameraData* camera, const RenderData* entrypoints, const RenderData* exitpoints, const LightSourceData* light) {
        cgtAssert(_rcShader != nullptr, "EEP Shader must not be 0.");

        // little hack to support LOD texture lookup for the gradients:
        // if texture does not yet have mipmaps, create them.
        const cgt::Texture* tex = images.front()->getTexture();
        if (tex->getFilter() != cgt::Texture::MIPMAP) {
            const_cast<cgt::Texture*>(tex)->setFilter(cgt::Texture::MIPMAP);
            glGenerateMipmap(GL_TEXTURE_3D);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
            LGL_ERROR;
        }

        _rcShader->activate();

        decorateRenderProlog(dataContainer, _rcShader);
        _rcShader->setUniform("_viewportSizeRCP", 1.f / cgt::vec2(getEffectiveViewportSize()));
        _rcShader->setUniform("_jitterStepSizeMultiplier", p_jitterStepSizeMultiplier.getValue());

        // compute sampling step size relative to volume size
        float samplingStepSize = .1f / p_samplingRate.getValue();
        _rcShader->setUniform("_samplingStepSize", samplingStepSize);

        // compute and set camera parameters
        const cgt::Camera& cam = camera->getCamera();
        float n = cam.getNearDist();
        float f = cam.getFarDist();
        _rcShader->setUniform("_cameraPosition", cam.getPosition());
        _rcShader->setUniform("const_to_z_e_1", 0.5f + 0.5f*((f+n)/(f-n)));
        _rcShader->setUniform("const_to_z_e_2", ((f-n)/(f*n)));
        _rcShader->setUniform("const_to_z_w_1", ((f*n)/(f-n)));
        _rcShader->setUniform("const_to_z_w_2", 0.5f*((f+n)/(f-n))+0.5f);

        // bind input textures
        cgt::TextureUnit volumeUnit, entryUnit, entryUnitDepth, exitUnit, exitUnitDepth, tf1Unit, tf2Unit, tf3Unit;
        images[0]->bind(_rcShader, volumeUnit, "_volume1", "_volumeParams1");
        images[1]->bind(_rcShader, volumeUnit, "_volume2", "_volumeParams2");
        images[2]->bind(_rcShader, volumeUnit, "_volume3", "_volumeParams3");
        p_transferFunction1.getTF()->bind(_rcShader, tf1Unit, "_transferFunction1", "_transferFunctionParams1");
        p_transferFunction2.getTF()->bind(_rcShader, tf2Unit, "_transferFunction2", "_transferFunctionParams2");
        p_transferFunction3.getTF()->bind(_rcShader, tf3Unit, "_transferFunction3", "_transferFunctionParams3");
        entrypoints->bind(_rcShader, entryUnit, entryUnitDepth, "_entryPoints", "_entryPointsDepth", "_entryParams");
        exitpoints->bind(_rcShader, exitUnit, exitUnitDepth, "_exitPoints", "_exitPointsDepth", "_exitParams");
        light->bind(_rcShader, "_lightSource");

        FramebufferActivationGuard fag(this);
        createAndAttachTexture(GL_RGBA8);
        createAndAttachTexture(GL_RGBA32F);
        createAndAttachTexture(GL_RGBA32F);
        createAndAttachDepthTexture();

        static const GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 , GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, buffers);

        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        QuadRdr.renderQuad();

        // restore state
        glDrawBuffers(1, buffers);
        glDisable(GL_DEPTH_TEST);

        decorateRenderEpilog(_rcShader);
        _rcShader->deactivate();

        LGL_ERROR;
        return new RenderData(_fbo);
    }

}
}
