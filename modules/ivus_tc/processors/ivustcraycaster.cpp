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

#include "ivustcraycaster.h"

#include "tgt/textureunit.h"

#include "core/tools/quadrenderer.h"
#include "core/datastructures/lightsourcedata.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorgradient.h"

namespace campvis {
    const std::string IvusTcRaycaster::loggerCat_ = "CAMPVis.modules.vis.IvusTcRaycaster";

    IvusTcRaycaster::IvusTcRaycaster(IVec2Property* viewportSizeProp)
        : RaycastingProcessor(viewportSizeProp, "modules/ivus_tc/glsl/ivustcraycaster.frag", true, "400")
        , p_cmImage("CmImage", "Confidence Map Image", "image.cm", DataNameProperty::READ)
        , p_tcImage("TcImage", "Tissue Classification Image", "image.tc", DataNameProperty::READ)
        , p_plaqueImage("PlaqueImage", "Plaque Mask Image", "image.plaque", DataNameProperty::READ)
        , p_enableShading("EnableShading", "Enable Shading", true)
        , p_lightId("LightId", "Input Light Source", "lightsource", DataNameProperty::READ)
        , p_predicateHistogram("PredicateHistogram", "Voxel Predicate Selection")
    {
        addDecorator(new ProcessorDecoratorGradient());

        addProperty(p_cmImage);
        addProperty(p_tcImage);
        addProperty(p_plaqueImage);

        addProperty(p_enableShading, INVALID_RESULT | INVALID_PROPERTIES | INVALID_SHADER);
        addProperty(p_lightId);
        addProperty(p_predicateHistogram);

        decoratePropertyCollection(this);
    }

    IvusTcRaycaster::~IvusTcRaycaster() {
    }

    void IvusTcRaycaster::init() {
        p_predicateHistogram.getPredicateHistogram()->setPredicateFunctionArgumentString("in float ivus, in float cm, in vec4 tc, in float plaque");

        RaycastingProcessor::init();
        p_predicateHistogram.s_headerChanged.connect(this, &IvusTcRaycaster::onHistogramHeaderChanged);
    }

    void IvusTcRaycaster::deinit() {
        p_predicateHistogram.s_headerChanged.disconnect(this);
        RaycastingProcessor::deinit();
    }

    void IvusTcRaycaster::processImpl(DataContainer& dataContainer, ImageRepresentationGL::ScopedRepresentation& image) {
        ImageRepresentationGL::ScopedRepresentation cm(dataContainer, p_cmImage.getValue());
        ImageRepresentationGL::ScopedRepresentation tc(dataContainer, p_tcImage.getValue());
        ImageRepresentationGL::ScopedRepresentation plaque(dataContainer, p_plaqueImage.getValue());

        if (cm && tc && plaque) {
            ScopedTypedData<LightSourceData> light(dataContainer, p_lightId.getValue());

            if (p_enableShading.getValue() == false || light != nullptr) {
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

                tgt::TextureUnit cmUnit, tcUnit, plaqueUnit;
                cm->bind(_shader, cmUnit, "_cm", "_cmParams");
                tc->bind(_shader, tcUnit, "_tc", "_tcParams");
                plaque->bind(_shader, plaqueUnit, "_plaque", "_plaqueParams");
                
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
            LDEBUG("Could not load Voxel Predicate Mask Image.");
        }
    }

    std::string IvusTcRaycaster::generateHeader() const {
        std::string toReturn = RaycastingProcessor::generateHeader();
        if (p_enableShading.getValue())
            toReturn += "#define ENABLE_SHADING\n";

        toReturn += p_predicateHistogram.getPredicateHistogram()->getGlslHeader();
        return toReturn;
    }

    void IvusTcRaycaster::onHistogramHeaderChanged() {
        invalidate(INVALID_SHADER);
    }

    void IvusTcRaycaster::updateProperties(DataContainer& dataContainer) {
        p_lightId.setVisible(p_enableShading.getValue());
        RaycastingProcessor::updateProperties(dataContainer);
        validate(INVALID_PROPERTIES);
    }

}
