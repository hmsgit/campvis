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

#include "ipsviraycaster.h"

#include "core/tools/quadrenderer.h"
#include "core/datastructures/cameradata.h"
#include "core/datastructures/lightsourcedata.h"
#include "core/datastructures/renderdata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/pipeline/processordecoratorgradient.h"

#include "modules/vis/tools/voxelhierarchymapper.h"

#include <tbb/tbb.h>

namespace campvis {
    const std::string IpsviRaycaster::loggerCat_ = "CAMPVis.modules.vis.IpsviRaycaster";

    IpsviRaycaster::IpsviRaycaster(IVec2Property* viewportSizeProp)
        : RaycastingProcessor(viewportSizeProp, "modules/vis/glsl/ipsviraycaster.frag", true, "440")
        , p_lightId("LightId", "Input Light Source", "lightsource", DataNameProperty::READ)
        , p_sweepLineWidth("SweepLineWidth", "Sweep Line Width", 2, 1, 32)
        , p_icTextureSize("IcTextureSize", "Illumination Cache Texture Size", cgt::ivec2(512), cgt::ivec2(32), cgt::ivec2(2048))
        , p_shadowIntensity("ShadowIntensity", "Shadow Intensity", .9f, 0.f, 1.f)
        , p_numLines("NumLines", "Max Number of Lines", 2000, 1, 2000)
        , _vhm(nullptr)
    {
        _icTextures[0] = nullptr;
        _icTextures[1] = nullptr;

        addProperty(p_lightId);
        addProperty(p_sweepLineWidth);
        addProperty(p_icTextureSize, INVALID_RESULT | INVALID_IC_TEXTURES);
        addProperty(p_shadowIntensity);
        addProperty(p_numLines);

        setPropertyInvalidationLevel(p_transferFunction, INVALID_BBV | INVALID_RESULT);
        setPropertyInvalidationLevel(p_sourceImageID, INVALID_BBV | INVALID_RESULT);

        addDecorator(new ProcessorDecoratorGradient());
        decoratePropertyCollection(this);
    }

    IpsviRaycaster::~IpsviRaycaster() {
    }

    void IpsviRaycaster::init() {
        RaycastingProcessor::init();

        _vhm = new VoxelHierarchyMapper();
        invalidate(INVALID_BBV | INVALID_IC_TEXTURES);
    }

    void IpsviRaycaster::deinit() {
        delete _vhm;
        delete _icTextures[0];
        delete _icTextures[1];

        RaycastingProcessor::deinit();
    }

    void IpsviRaycaster::processImpl(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image) {
        if (getInvalidationLevel() & INVALID_IC_TEXTURES) {
            delete _icTextures[0];
            delete _icTextures[1];

            cgt::ivec3 icSize(p_icTextureSize.getValue(), 1);
            cgt::TextureUnit icUnit;

            icUnit.activate();
            _icTextures[0] = new cgt::Texture(GL_TEXTURE_2D, icSize, GL_R32F);
            _icTextures[1] = new cgt::Texture(GL_TEXTURE_2D, icSize, GL_R32F);

            validate(INVALID_IC_TEXTURES);
        }

        if (getInvalidationLevel() & INVALID_BBV) {
            _shader->deactivate();
            _vhm->createHierarchy(image, p_transferFunction.getTF());
            _shader->activate();

            validate(INVALID_BBV);
        }

        if (_vhm->getHierarchyTexture() == nullptr) {
            LERROR("Could not retreive voxel hierarchy lookup structure.");
            return;
        }

        ScopedTypedData<CameraData> camera(data, p_camera.getValue());
        ScopedTypedData<LightSourceData> light(data, p_lightId.getValue());

        if (camera && light) {
            processDirectional(data, image, *camera, *light);
        }
        else {
            LDEBUG("Could not load light source from DataContainer.");
        }
    }

    void IpsviRaycaster::processDirectional(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image, const CameraData& camera, const LightSourceData& light) {
        const cgt::vec3& lightSink = camera.getCamera().getFocus();
        const cgt::vec3 lightSource = camera.getCamera().getFocus() + light.getLightPosition();
        const cgt::vec3& lightDirection = light.getLightPosition();

        // TODO: This should be a world to NDC space conversion, but it does not work... :(
        const auto V = camera.getCamera().getViewMatrix();
        const auto P = camera.getCamera().getProjectionMatrix();

        // calculate viewport matrix for NDC -> viewport conversion
        const cgt::vec2 halfViewport = cgt::vec2(getEffectiveViewportSize()) / 2.f;
        const cgt::mat4 viewportMatrix = cgt::mat4::createTranslation(cgt::vec3(halfViewport, 0.f)) * cgt::mat4::createScale(cgt::vec3(halfViewport, 1.f));

        const cgt::vec4 projectedLight = viewportMatrix*P*V* cgt::vec4(lightSource, 1.f);
        const cgt::vec4 projectedOrigin = viewportMatrix*P*V* cgt::vec4(lightSink, 1.f);
        cgt::vec2 projectedLightDirection = projectedOrigin.xy()/projectedOrigin.w - projectedLight.xy()/projectedLight.w;

        // compute sweep direction (in viewport space)
        enum SweepDirection { LeftToRight, RightToLeft, TopToBottom, BottomToTop };
        SweepDirection sweepDir;
        if (std::abs(projectedLightDirection.x) > std::abs(projectedLightDirection.y)) {
            // horizontal sweep
            if (projectedLightDirection.x < 0)
                sweepDir = LeftToRight;
            else
                sweepDir = RightToLeft;
        }
        else {
            // vertical sweep
            if (projectedLightDirection.y < 0)
                sweepDir = BottomToTop;
            else
                sweepDir = TopToBottom;
        }

        // START: compute illumination cache (IC) plane/texture
        // the plane is defined by the light direction
        cgt::vec3 icNormal = cgt::normalize(lightDirection);
        cgt::vec3 icUpVector = (std::abs(cgt::dot(icNormal, cgt::vec3(0.f, 0.f, 1.f))) < 0.99) ? cgt::vec3(0.f, 0.f, 1.f) : cgt::vec3(0.f, 1.f, 0.f);
        cgt::vec3 icRightVector = cgt::normalize(cgt::cross(icNormal, icUpVector));
        icUpVector = cgt::normalize(cgt::cross(icRightVector, icNormal));

        // project all 8 corners of the volume onto the IC plane
        cgt::Bounds worldBounds = image->getParent()->getWorldBounds();
        cgt::Bounds viewportBounds;
        cgt::vec3 minPixel(0.f), maxPixel(0.f);
        std::vector<cgt::vec3> corners;
        corners.push_back(cgt::vec3(worldBounds.getLLF().x, worldBounds.getLLF().y, worldBounds.getLLF().z));
        corners.push_back(cgt::vec3(worldBounds.getLLF().x, worldBounds.getLLF().y, worldBounds.getURB().z));
        corners.push_back(cgt::vec3(worldBounds.getLLF().x, worldBounds.getURB().y, worldBounds.getLLF().z));
        corners.push_back(cgt::vec3(worldBounds.getLLF().x, worldBounds.getURB().y, worldBounds.getURB().z));
        corners.push_back(cgt::vec3(worldBounds.getURB().x, worldBounds.getLLF().y, worldBounds.getLLF().z));
        corners.push_back(cgt::vec3(worldBounds.getURB().x, worldBounds.getLLF().y, worldBounds.getURB().z));
        corners.push_back(cgt::vec3(worldBounds.getURB().x, worldBounds.getURB().y, worldBounds.getLLF().z));
        corners.push_back(cgt::vec3(worldBounds.getURB().x, worldBounds.getURB().y, worldBounds.getURB().z));

        for (auto i = 0; i < corners.size(); ++i) {
            const cgt::vec3 diag = corners[i];
            const float distance = std::abs(cgt::dot(diag, icNormal));
            const cgt::vec3 projected = diag - (-distance * icNormal);
            const cgt::vec3 pixel(cgt::dot(projected, icRightVector), cgt::dot(projected, icUpVector), 0.f);

            minPixel = cgt::min(minPixel, pixel);
            maxPixel = cgt::max(maxPixel, pixel);

            // project onto viewport to calculate viewport extent
            const cgt::vec4 viewportPixel = viewportMatrix*P*V* cgt::vec4(corners[i], 1.f);
            viewportBounds.addPoint(viewportPixel.xyz() / viewportPixel.w);
        }

        cgt::vec3 icOrigin = cgt::floor(minPixel).x * icRightVector + cgt::floor(minPixel).y * icUpVector;
        cgt::ivec3 icSize(p_icTextureSize.getValue(), 1);
        icRightVector *= float(icSize.x - 1) / (std::ceil(maxPixel.x) - std::floor(minPixel.x)) ;
        icUpVector *= float(icSize.y - 1) / (std::ceil(maxPixel.y) - std::floor(minPixel.y));

        // bind voxel hierarchy to shader
        cgt::TextureUnit xorUnit, bbvUnit;
        {
            cgt::Shader::IgnoreUniformLocationErrorGuard guard(_shader);

            xorUnit.activate();
            _vhm->getXorBitmaskTexture()->bind();
            _shader->setUniform("_xorBitmask", xorUnit.getUnitNumber());

            bbvUnit.activate();
            _vhm->getHierarchyTexture()->bind();
            _shader->setUniform("_voxelHierarchy", bbvUnit.getUnitNumber());
            _shader->setUniform("_vhMaxMipMapLevel", static_cast<int>(_vhm->getMaxMipmapLevel()));
        
        }

        // clear and bind IC textures
        cgt::col4 zeroInit(0, 0, 0, 0);
        glClearTexImage(_icTextures[0]->getId(), 0, GL_RED, GL_UNSIGNED_BYTE, &zeroInit);
        glClearTexImage(_icTextures[1]->getId(), 0, GL_RED, GL_UNSIGNED_BYTE, &zeroInit);
        glBindImageTexture(0, _icTextures[0]->getId(), 0, false, 0, GL_READ_WRITE, GL_R32F);
        glBindImageTexture(1, _icTextures[1]->getId(), 0, false, 0, GL_READ_WRITE, GL_R32F);

        // setup IC shader uniforms
        _shader->setUniform("_icOrigin", icOrigin);
        _shader->setUniform("_icNormal", icNormal);
        _shader->setUniform("_icRightVector", icRightVector);
        _shader->setUniform("_icUpVector", icUpVector);
        _shader->setUniform("_shadowIntensity", p_shadowIntensity.getValue());


        FramebufferActivationGuard fag(this);
        createAndAttachTexture(GL_RGBA8);
        createAndAttachTexture(GL_RGBA32F);
        createAndAttachTexture(GL_RGBA32F);
        createAndAttachDepthTexture();

        static const GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 , GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, buffers);

        light.bind(_shader, "_lightSource");

        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        auto viewportSize = getEffectiveViewportSize();

        cgt::mat4 projection;
        cgt::mat4 viewScale;
        cgt::vec3 viewTranslationBase;
        int line = 1 - p_sweepLineWidth.getValue();
        int lineMax;
        float scale = 1.f;
        float bias = 0.f;

        switch (sweepDir) {
        case LeftToRight:
            scale = float(viewportSize.y) / viewportBounds.diagonal().y;
            bias = viewportBounds.getLLF().y / float(viewportSize.y) * scale;
            projection = cgt::mat4::createOrtho(0, viewportSize.x, scale-bias, -bias, -1, 1);
            viewScale = cgt::mat4::createScale(cgt::vec3(p_sweepLineWidth.getValue(), 1.f, 1.f));
            viewTranslationBase = cgt::vec3(1.f, 0.f, 0.f);
            line += std::max(0, int(viewportBounds.getLLF().x));
            lineMax = std::min(viewportSize.x, int(viewportBounds.getURB().x));
            break;
        case RightToLeft:
            scale = float(viewportSize.y) / viewportBounds.diagonal().y;
            bias = viewportBounds.getLLF().y / float(viewportSize.y) * scale;
            projection = cgt::mat4::createOrtho(viewportSize.x, 0, scale-bias, -bias, -1, 1);
            viewScale = cgt::mat4::createScale(cgt::vec3(p_sweepLineWidth.getValue(), 1.f, 1.f));
            viewTranslationBase = cgt::vec3(1.f, 0.f, 0.f);
            line += std::max(0, viewportSize.x - int(viewportBounds.getURB().x));
            lineMax = std::min(viewportSize.x, viewportSize.x - int(viewportBounds.getLLF().x));
            break;
        case BottomToTop:
            scale = float(viewportSize.x) / viewportBounds.diagonal().x;
            bias = viewportBounds.getLLF().x / float(viewportSize.x) * scale;
            projection = cgt::mat4::createOrtho(-bias, scale-bias, viewportSize.y, 0, -1, 1);
            viewScale = cgt::mat4::createScale(cgt::vec3(1.f, p_sweepLineWidth.getValue(), 1.f));
            viewTranslationBase = cgt::vec3(0.f, 1.f, 0.f);
            line += std::max(0, int(viewportBounds.getLLF().y));
            lineMax = std::min(viewportSize.y, int(viewportBounds.getURB().y));
            break;
        case TopToBottom:
            scale = float(viewportSize.x) / viewportBounds.diagonal().x;
            bias = viewportBounds.getLLF().x / float(viewportSize.x) * scale;
            projection = cgt::mat4::createOrtho(-bias, scale-bias, 0, viewportSize.y, -1, 1);
            viewScale = cgt::mat4::createScale(cgt::vec3(1.f, p_sweepLineWidth.getValue(), 1.f));
            viewTranslationBase = cgt::vec3(0.f, 1.f, 0.f);
            line += std::max(0, viewportSize.y - int(viewportBounds.getURB().y));
            lineMax = std::min(viewportSize.y, viewportSize.y - int(viewportBounds.getLLF().y));
            break;
        }


        int evenOdd = 0;

        _shader->setUniform("_projectionMatrix", projection);
        GLint uIcImageIn = _shader->getUniformLocation("_icImageIn");
        GLint uIcImageOut = _shader->getUniformLocation("_icImageOut");
        GLint uViewMatrix = _shader->getUniformLocation("_viewMatrix");            

        while (line < lineMax) {
            // ping-pong buffering to avoid concurrent read-writes
            if (evenOdd % 2 == 0) {
                _shader->setUniform(uIcImageIn, 0);
                _shader->setUniform(uIcImageOut, 1);
            }
            else {
                _shader->setUniform(uIcImageIn, 1);
                _shader->setUniform(uIcImageOut, 0);
            }

            _shader->setUniform(uViewMatrix, cgt::mat4::createTranslation(viewTranslationBase * float(line)) * viewScale);
            QuadRdr.renderQuad01();

            line += p_sweepLineWidth.getValue();
            ++evenOdd;
            glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);

            if (evenOdd > p_numLines.getValue())
                break;
        }

        // restore state
        glDrawBuffers(1, buffers);
        glDisable(GL_DEPTH_TEST);
        glBindImageTexture(0, 0, 0, false, 0, GL_READ_ONLY, GL_R32F);
        glBindImageTexture(1, 0, 0, false, 0, GL_READ_ONLY, GL_R32F);
        LGL_ERROR;

        data.addData(p_targetImageID.getValue(), new RenderData(_fbo));
    }

    void IpsviRaycaster::processPointLight(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image, const CameraData& camera, const LightSourceData& light) {
        // calculate viewport matrix for NDC -> viewport conversion
        const auto& V = camera.getCamera().getViewMatrix();
        const auto& P = camera.getCamera().getProjectionMatrix();
        const cgt::vec2 halfViewport = cgt::vec2(getEffectiveViewportSize()) / 2.f;
        const cgt::mat4 viewportMatrix = cgt::mat4::createTranslation(cgt::vec3(halfViewport, 0.f)) * cgt::mat4::createScale(cgt::vec3(halfViewport, 1.f));

        // project light position to viewport coordinates
        cgt::vec4 projectedLight = viewportMatrix * P * V * cgt::vec4(light.getLightPosition(), 1.f);
        projectedLight /= projectedLight.w;

    }

}
