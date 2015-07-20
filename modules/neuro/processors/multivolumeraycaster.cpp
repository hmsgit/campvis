// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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
        , p_sourceImage1("SourceImage1", "First Input Image", "", DataNameProperty::READ)
        , p_sourceImage2("SourceImage2", "Second Input Image", "", DataNameProperty::READ)
        , p_sourceImage3("SourceImage3", "Third Input Image", "", DataNameProperty::READ)
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
        , _rcShader(nullptr)
    {
        addDecorator(new ProcessorDecoratorGradient());

        addProperty(p_sourceImage1, INVALID_PROPERTIES | INVALID_RESULT | INVALID_VOXEL_HIERARCHY1);
        addProperty(p_sourceImage2, INVALID_PROPERTIES | INVALID_RESULT | INVALID_VOXEL_HIERARCHY2);
        addProperty(p_sourceImage3, INVALID_PROPERTIES | INVALID_RESULT | INVALID_VOXEL_HIERARCHY3);
        addProperty(p_geometryImageId);
        addProperty(p_camera);
        addProperty(p_lightId);
        addProperty(p_outputImageId);

        addProperty(p_transferFunction1, INVALID_RESULT | INVALID_VOXEL_HIERARCHY1);
        addProperty(p_transferFunction2, INVALID_RESULT | INVALID_VOXEL_HIERARCHY2);
        addProperty(p_transferFunction3, INVALID_RESULT | INVALID_VOXEL_HIERARCHY3);
        addProperty(p_jitterStepSizeMultiplier);
        addProperty(p_samplingRate);

        decoratePropertyCollection(this);
    }

    MultiVolumeRaycaster::~MultiVolumeRaycaster() {

    }

    void MultiVolumeRaycaster::init() {
        VisualizationProcessor::init();

        _eepShader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/neuro/glsl/multivolumeraycaster_eep.frag", generateHeader());
        _rcShader = ShdrMgr.loadWithCustomGlslVersion("core/glsl/passthrough.vert", "", "modules/neuro/glsl/multivolumeraycaster_rc.frag", generateHeader(), "400");

        _vhm1 = new VoxelHierarchyMapper();
        _vhm2 = new VoxelHierarchyMapper();
        _vhm3 = new VoxelHierarchyMapper();
    }

    void MultiVolumeRaycaster::deinit() {
        ShdrMgr.dispose(_eepShader);
        _eepShader = nullptr;
        ShdrMgr.dispose(_rcShader);
        _rcShader = nullptr;

        delete _vhm1;
        delete _vhm2;
        delete _vhm3;

        VisualizationProcessor::deinit();
    }

    void MultiVolumeRaycaster::updateResult(DataContainer& dataContainer) {
        ImageRepresentationGL::ScopedRepresentation image1(dataContainer, p_sourceImage1.getValue());
        ImageRepresentationGL::ScopedRepresentation image2(dataContainer, p_sourceImage2.getValue());
        ImageRepresentationGL::ScopedRepresentation image3(dataContainer, p_sourceImage3.getValue());
        ScopedTypedData<CameraData> camera(dataContainer, p_camera.getValue());
        ScopedTypedData<RenderData> geometryImage(dataContainer, p_geometryImageId.getValue(), true);
        ScopedTypedData<LightSourceData> light(dataContainer, p_lightId.getValue());

        std::vector<const ImageRepresentationGL*> images;
        if (image1) {
            images.push_back(image1);

            if (getInvalidationLevel() & INVALID_VOXEL_HIERARCHY1){
                _vhm1->createHierarchy(image1, p_transferFunction1.getTF());
                validate(INVALID_VOXEL_HIERARCHY1);
            }
        }
        if (image2) {
            images.push_back(image2);

            if (getInvalidationLevel() & INVALID_VOXEL_HIERARCHY2){
                _vhm2->createHierarchy(image2, p_transferFunction2.getTF());
                validate(INVALID_VOXEL_HIERARCHY2);
            }
        }
        if (image3) {
            images.push_back(image3);

            if (getInvalidationLevel() & INVALID_VOXEL_HIERARCHY3){
                _vhm3->createHierarchy(image3, p_transferFunction3.getTF());
                validate(INVALID_VOXEL_HIERARCHY3);
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

    void MultiVolumeRaycaster::updateShader() {
        _eepShader->setHeaders(generateHeader());
        _eepShader->rebuild();
        _rcShader->setHeaders(generateHeader());
        _rcShader->rebuild();
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

        _rcShader->activate();

        decorateRenderProlog(dataContainer, _rcShader);
        _rcShader->setUniform("_viewportSizeRCP", 1.f / cgt::vec2(getEffectiveViewportSize()));
        _rcShader->setUniform("_jitterStepSizeMultiplier", p_jitterStepSizeMultiplier.getValue());

        // compute sampling step size relative to volume size
        float samplingStepSize = .001f / p_samplingRate.getValue();
        if (p_lqMode.getValue())
            samplingStepSize *= 4.f;
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
        cgt::TextureUnit volumeUnit1, volumeUnit2, volumeUnit3, entryUnit, entryUnitDepth, exitUnit, exitUnitDepth, tf1Unit, tf2Unit, tf3Unit;
        images[0]->bind(_rcShader, volumeUnit1, "_volume1", "_volumeParams1");
        images[1]->bind(_rcShader, volumeUnit2, "_volume2", "_volumeParams2");
        images[2]->bind(_rcShader, volumeUnit3, "_volume3", "_volumeParams3");
        p_transferFunction1.getTF()->bind(_rcShader, tf1Unit, "_transferFunction1", "_transferFunctionParams1");
        p_transferFunction2.getTF()->bind(_rcShader, tf2Unit, "_transferFunction2", "_transferFunctionParams2");
        p_transferFunction3.getTF()->bind(_rcShader, tf3Unit, "_transferFunction3", "_transferFunctionParams3");
        entrypoints->bind(_rcShader, entryUnit, entryUnitDepth, "_entryPoints", "_entryPointsDepth", "_entryParams");
        exitpoints->bind(_rcShader, exitUnit, exitUnitDepth, "_exitPoints", "_exitPointsDepth", "_exitParams");
        light->bind(_rcShader, "_lightSource");

        // bind voxel hierarchies
        cgt::TextureUnit xorUnit, vhUnit1, vhUnit2, vhUnit3;
        xorUnit.activate();
        _vhm1->getXorBitmaskTexture()->bind();
        _rcShader->setUniform("_xorBitmask", xorUnit.getUnitNumber());

        vhUnit1.activate();
        _vhm1->getHierarchyTexture()->bind();
        _rcShader->setUniform("_voxelHierarchy1", vhUnit1.getUnitNumber());
        _rcShader->setUniform("_vhMaxMipMapLevel1", static_cast<int>(_vhm1->getMaxMipmapLevel()));

        if (_vhm2) {
            vhUnit2.activate();
            _vhm2->getHierarchyTexture()->bind();
            _rcShader->setUniform("_voxelHierarchy2", vhUnit2.getUnitNumber());
            _rcShader->setUniform("_vhMaxMipMapLevel2", static_cast<int>(_vhm2->getMaxMipmapLevel()));
        }

        if (_vhm3) {
            vhUnit3.activate();
            _vhm3->getHierarchyTexture()->bind();
            _rcShader->setUniform("_voxelHierarchy3", vhUnit3.getUnitNumber());
            _rcShader->setUniform("_vhMaxMipMapLevel3", static_cast<int>(_vhm3->getMaxMipmapLevel()));
        }

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
