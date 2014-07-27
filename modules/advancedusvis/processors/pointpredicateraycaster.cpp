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

#include "pointpredicateraycaster.h"

#include "tgt/textureunit.h"

#include "core/tools/quadrenderer.h"
#include "core/datastructures/lightsourcedata.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorgradient.h"

namespace campvis {
    const std::string PointPredicateRaycaster::loggerCat_ = "CAMPVis.modules.vis.PointPredicateRaycaster";

    PointPredicateRaycaster::PointPredicateRaycaster(IVec2Property* viewportSizeProp)
        : RaycastingProcessor(viewportSizeProp, "modules/advancedusvis/glsl/pointpredicateraycaster.frag", true, "400")
        , p_inputLabels("InputLabels", "Input Label Image", "", DataNameProperty::READ)
        , p_inputSnr("InputSnr", "Input SNR", "", DataNameProperty::READ)
        , p_inputVesselness("InputVesselness", "Input Vesselness", "", DataNameProperty::READ)
        , p_inputConfidence("InputConfidence", "Input Confidence", "", DataNameProperty::READ)
        , p_enableShading("EnableShading", "Enable Shading", true)
        , p_lightId("LightId", "Input Light Source", "lightsource", DataNameProperty::READ)
        , p_predicateHistogram("PredicateSelection", "Voxel Predicate Selection")
    {
        addDecorator(new ProcessorDecoratorGradient());

        addProperty(p_inputLabels, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_inputSnr);
        addProperty(p_inputVesselness);
        addProperty(p_inputConfidence);

        addProperty(p_enableShading, INVALID_RESULT | INVALID_PROPERTIES | INVALID_SHADER);
        addProperty(p_lightId);
        addProperty(p_predicateHistogram);

        decoratePropertyCollection(this);
    }

    PointPredicateRaycaster::~PointPredicateRaycaster() {
    }

    void PointPredicateRaycaster::init() {
        p_predicateHistogram.s_headerChanged.connect(this, &PointPredicateRaycaster::onHistogramHeaderChanged);
        RaycastingProcessor::init();
    }

    void PointPredicateRaycaster::deinit() {
        p_predicateHistogram.s_headerChanged.disconnect(this);
        RaycastingProcessor::deinit();
    }

    void PointPredicateRaycaster::processImpl(DataContainer& dataContainer, ImageRepresentationGL::ScopedRepresentation& image) {
        ImageRepresentationGL::ScopedRepresentation labels(dataContainer, p_inputLabels.getValue());
        ImageRepresentationGL::ScopedRepresentation snr(dataContainer, p_inputSnr.getValue());
        ImageRepresentationGL::ScopedRepresentation vesselness(dataContainer, p_inputVesselness.getValue());
        ImageRepresentationGL::ScopedRepresentation confidence(dataContainer, p_inputConfidence.getValue());

        if (labels && snr && vesselness && confidence) {
            ScopedTypedData<LightSourceData> light(dataContainer, p_lightId.getValue());

            if (p_enableShading.getValue() == false || light != nullptr) {
                const tgt::Texture* lt = labels->getTexture();
                if (lt->getFilter() != tgt::Texture::NEAREST) {
                    const_cast<tgt::Texture*>(lt)->setFilter(tgt::Texture::NEAREST);
                }

                const tgt::Texture* tex = image->getTexture();
                if (tex->getFilter() != tgt::Texture::MIPMAP) {
                    const_cast<tgt::Texture*>(tex)->setFilter(tgt::Texture::MIPMAP);
                    LGL_ERROR;
                    glGenerateMipmap(GL_TEXTURE_3D);
                    LGL_ERROR;
                    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    LGL_ERROR;
                    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    LGL_ERROR;
                }

                tgt::TextureUnit labelUnit, snrUnit, vesselnessUnit, confidenceUnit;
                labels->bind(_shader, labelUnit, "_labels", "_labelsParams");
                snr->bind(_shader, snrUnit, "_snr", "_snrParams");
                vesselness->bind(_shader, vesselnessUnit, "_vesselness", "_vesselnessParams");
                confidence->bind(_shader, confidenceUnit, "_confidence", "_confidenceParams");
                
                if (p_enableShading.getValue() && light != nullptr) {
                    light->bind(_shader, "_lightSource");
                }

                _shader->setIgnoreUniformLocationError(true);
                p_predicateHistogram.getPredicateHistogram()->setupRenderShader(_shader);
                _shader->setIgnoreUniformLocationError(false);
                LGL_ERROR;

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
                LGL_ERROR;

                dataContainer.addData(p_targetImageID.getValue(), new RenderData(_fbo));
            }
            else {
                LDEBUG("Could not load light source from DataContainer.");
            }
        }
        else {
            LERROR("Could not load Voxel Predicate Mask Image.");
        }
    }

    std::string PointPredicateRaycaster::generateHeader() const {
        std::string toReturn = RaycastingProcessor::generateHeader();
        if (p_enableShading.getValue())
            toReturn += "#define ENABLE_SHADING\n";

        toReturn += p_predicateHistogram.getPredicateHistogram()->getGlslHeader();
        return toReturn;
    }

    void PointPredicateRaycaster::onHistogramHeaderChanged() {
        invalidate(INVALID_SHADER);
    }

    void PointPredicateRaycaster::updateProperties(DataContainer& dataContainer) {
        p_lightId.setVisible(p_enableShading.getValue());
        RaycastingProcessor::updateProperties(dataContainer);
    }

}
