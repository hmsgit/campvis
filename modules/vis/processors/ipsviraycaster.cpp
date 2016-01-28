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

#include <tbb/tbb.h>

namespace campvis {
    const std::string IpsviRaycaster::loggerCat_ = "CAMPVis.modules.vis.IpsviRaycaster";

    IpsviRaycaster::IpsviRaycaster(IVec2Property* viewportSizeProp)
        : RaycastingProcessor(viewportSizeProp, "modules/vis/glsl/ipsviraycaster.frag", true, "450")
        , p_lightId("LightId", "Input Light Source", "lightsource", DataNameProperty::READ)
        , p_sweepLineWidth("SweepLineWidth", "Sweep Line Width", 2, 1, 8)
        , p_shadowIntensity("ShadowIntensity", "Shadow Intensity", .9f, 0.f, 1.f)
    {
        addProperty(p_lightId);
        addProperty(p_sweepLineWidth);
        addProperty(p_shadowIntensity);

        addDecorator(new ProcessorDecoratorGradient());
        decoratePropertyCollection(this);
    }

    IpsviRaycaster::~IpsviRaycaster() {
    }

    void IpsviRaycaster::init() {
        RaycastingProcessor::init();
    }

    void IpsviRaycaster::deinit() {
        RaycastingProcessor::deinit();
    }

    void IpsviRaycaster::processImpl(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image) {
        ScopedTypedData<CameraData> camera(data, p_camera.getValue());
        ScopedTypedData<LightSourceData> light(data, p_lightId.getValue());

        if (light != nullptr) {
            cgt::vec3 lightDirection = (light->getLightPosition());

            // TODO: This should be a world to NDC space conversion, but it does not work... :(
            const auto V = camera->getCamera().getViewMatrix();
            const auto P = camera->getCamera().getProjectionMatrix();

            // calculate viewport matrix for NDC -> viewport conversion
            const cgt::vec2 halfViewport = cgt::vec2(getEffectiveViewportSize()) / 2.f;
            const cgt::mat4 viewportMatrix = cgt::mat4::createTranslation(cgt::vec3(halfViewport, 0.f)) * cgt::mat4::createScale(cgt::vec3(halfViewport, 1.f));

            const cgt::vec4 projectedLight = viewportMatrix*P*V*cgt::vec4(lightDirection, 1.f);
            const cgt::vec4 projectedOrigin = viewportMatrix*P*V*cgt::vec4(0.f, 0.f, 0.f, 1.f);
            cgt::vec2 projectedLightDirection = projectedOrigin.xy()/projectedOrigin.w - projectedLight.xy()/projectedLight.w;
            
            // compute sweep direction (in viewport space)
            enum SweepDirection { LeftToRight, RightToLeft, TopToBottom, BottomToTop };
            SweepDirection sweepDir;
            if (std::abs(projectedLightDirection.x) > std::abs(projectedLightDirection.y)) {
                // horizontal sweep
                if (projectedLightDirection.x > 0)
                    sweepDir = LeftToRight;
                else
                    sweepDir = RightToLeft;
            }
            else {
                // vertical sweep
                if (projectedLightDirection.y > 0)
                    sweepDir = BottomToTop;
                else
                    sweepDir = TopToBottom;
            }

            // START: compute illumination cache (IC) plane/texture
            // the plane is defined by the light direction
            cgt::vec3 icNormal = cgt::normalize(lightDirection);
            cgt::vec3 icUpVector = (std::abs(cgt::dot(icNormal, cgt::vec3(0.f, 0.f, 1.f))) < 0.01) ? cgt::vec3(0.f, 0.f, 1.f) : cgt::vec3(0.f, 1.f, 0.f);
            cgt::vec3 icRightVector = cgt::normalize(cgt::cross(icNormal, icUpVector));
            icUpVector = cgt::normalize(cgt::cross(icRightVector, icNormal));

            // project all 8 corners of the volume onto the IC plane
            cgt::Bounds worldBounds = image->getParent()->getWorldBounds();
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
            }

            cgt::vec3 icOrigin = cgt::floor(minPixel).x * icRightVector + cgt::floor(minPixel).y * icUpVector;
            cgt::ivec3 icSize(384, 384, 1);
            icRightVector *= float(icSize.x - 1) / (std::ceil(maxPixel.x) - std::floor(minPixel.x)) ;
            icUpVector *= float(icSize.y - 1) / (std::ceil(maxPixel.y) - std::floor(minPixel.y));

            // just debuggin/asserting correctness
            for (auto i = 0; i < corners.size(); ++i) {
                const cgt::vec3 diag = corners[i] - icOrigin;
                const float distance = std::abs(cgt::dot(diag, icNormal));
                const cgt::vec3 projected = diag - (-distance * icNormal);
                const cgt::vec3 pixel(cgt::dot(projected, icRightVector), cgt::dot(projected, icUpVector), 0.f);

                if (pixel.x < 0.f || pixel.y < 0.f || pixel.x >= icSize.x || pixel.y >= icSize.y)
                    LWARNING(pixel);
            }

            cgt::col4* zeroInit = new cgt::col4[cgt::hmul(icSize)];
            memset(zeroInit, 0, sizeof(cgt::col4) * cgt::hmul(icSize));

            cgt::TextureUnit icUnit1, icUnit2;
            cgt::Texture* icTextures[2];
            icUnit1.activate();
            icTextures[0] = new cgt::Texture(GL_TEXTURE_2D, icSize, GL_RGBA32F, zeroInit->elem, GL_RGBA, GL_UNSIGNED_BYTE);
            icUnit2.activate();
            icTextures[1] = new cgt::Texture(GL_TEXTURE_2D, icSize, GL_RGBA32F, zeroInit->elem, GL_RGBA, GL_UNSIGNED_BYTE);

            glBindImageTexture(0, icTextures[0]->getId(), 0, false, 0, GL_READ_WRITE, GL_RGBA32F);
            glBindImageTexture(1, icTextures[1]->getId(), 0, false, 0, GL_READ_WRITE, GL_RGBA32F);

            delete [] zeroInit;

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

            light->bind(_shader, "_lightSource");

            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            auto viewportSize = getEffectiveViewportSize();

            cgt::mat4 projection;
            cgt::mat4 viewScale;
            cgt::vec3 viewTranslationBase;
            int lineMax;

            switch (sweepDir) {
                case LeftToRight:
                    projection = cgt::mat4::createOrtho(0, viewportSize.x, 0, 1, -1, 1);
                    viewScale = cgt::mat4::createScale(cgt::vec3(p_sweepLineWidth.getValue(), 1.f, 1.f));
                    viewTranslationBase = cgt::vec3(1.f, 0.f, 0.f);
                    lineMax = viewportSize.x;
                    break;
                case RightToLeft:
                    projection = cgt::mat4::createOrtho(viewportSize.x, 0, 0, 1, -1, 1);
                    viewScale = cgt::mat4::createScale(cgt::vec3(p_sweepLineWidth.getValue(), 1.f, 1.f));
                    viewTranslationBase = cgt::vec3(1.f, 0.f, 0.f);
                    lineMax = viewportSize.x;
                    break;
                case BottomToTop:
                    projection = cgt::mat4::createOrtho(0, 1, viewportSize.y, 0, -1, 1);
                    viewScale = cgt::mat4::createScale(cgt::vec3(1.f, p_sweepLineWidth.getValue(), 1.f));
                    viewTranslationBase = cgt::vec3(0.f, 1.f, 0.f);
                    lineMax = viewportSize.y;
                    break;
                case TopToBottom:
                    projection = cgt::mat4::createOrtho(0, 1, 0, viewportSize.y, -1, 1);
                    viewScale = cgt::mat4::createScale(cgt::vec3(1.f, p_sweepLineWidth.getValue(), 1.f));
                    viewTranslationBase = cgt::vec3(0.f, 1.f, 0.f);
                    lineMax = viewportSize.y;
                    break;
            }

            _shader->setUniform("_projectionMatrix", projection);

            int line = 1 - p_sweepLineWidth.getValue();
            int evenOdd = 0;

            while (line < lineMax) {
                if (evenOdd % 2 == 0) {
                    _shader->setUniform("_icImageIn", 0);
                    _shader->setUniform("_icImageOut", 1);
                }
                else {
                    _shader->setUniform("_icImageIn", 1);
                    _shader->setUniform("_icImageOut", 0);
                }

                _shader->setUniform("_viewMatrix", cgt::mat4::createTranslation(viewTranslationBase * float(line)) * viewScale);
                QuadRdr.renderQuad01();

                line += p_sweepLineWidth.getValue();
                ++evenOdd;
                glMemoryBarrier(GL_SHADER_IMAGE_ACCESS_BARRIER_BIT);
            }

            // restore state
            glDrawBuffers(1, buffers);
            glDisable(GL_DEPTH_TEST);
            glBindImageTexture(0, 0, 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
            glBindImageTexture(1, 0, 0, false, 0, GL_READ_ONLY, GL_RGBA32F);
            LGL_ERROR;

            data.addData(p_targetImageID.getValue(), new RenderData(_fbo));

            RenderData* ic = new RenderData();
            ImageData* id1 = new ImageData(2, icSize, 4);
            ImageRepresentationGL::create(id1, icTextures[0]);
            ic->addColorTexture(id1);
            ImageData* id2 = new ImageData(2, icSize, 4);
            ImageRepresentationGL::create(id2, icTextures[1]);
            ic->addColorTexture(id2);
            data.addData(p_targetImageID.getValue() + ".IC", ic);
        }
        else {
            LDEBUG("Could not load light source from DataContainer.");
        }
    }

}
