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

#include "ivustcsliceextractor.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"
#include "core/classification/simpletransferfunction.h"

#include "core/tools/quadrenderer.h"

namespace campvis {
    const std::string IvusTcSliceExtractor::loggerCat_ = "CAMPVis.modules.vis.IvusTcSliceExtractor";

    IvusTcSliceExtractor::IvusTcSliceExtractor(IVec2Property* viewportSizeProp)
        : SliceRenderProcessor(viewportSizeProp, "modules/ivus_tc/glsl/ivustcsliceextractor.frag", "400")
        , p_cmImage("CmImage", "Confidence Map Image", "image.cm", DataNameProperty::READ)
        , p_tcImage("TcImage", "Tissue Classification Image", "image.tc", DataNameProperty::READ)
        , p_plaqueImage("PlaqueImage", "Plaque Mask Image", "image.plaque", DataNameProperty::READ)
        , p_predicateHistogram("PredicateHistogram", "predicate Histogram")
        , p_mixWithIvus("MixWithIvus", "Mix Segmentation with IVUS", true)
    {
        addProperty(p_cmImage);
        addProperty(p_tcImage);
        addProperty(p_plaqueImage);
        addProperty(p_predicateHistogram);
        addProperty(p_mixWithIvus);

        invalidate(AbstractProcessor::INVALID_SHADER);

        p_predicateHistogram.getPredicateHistogram()->setPredicateFunctionArgumentString("in float ivus, in float cm, in vec4 tc, in float plaque");
    }

    IvusTcSliceExtractor::~IvusTcSliceExtractor() {

    }

    void IvusTcSliceExtractor::renderImageImpl(DataContainer& dataContainer, const ImageRepresentationGL::ScopedRepresentation& img) {
        ImageRepresentationGL::ScopedRepresentation cm(dataContainer, p_cmImage.getValue());
        ImageRepresentationGL::ScopedRepresentation tc(dataContainer, p_tcImage.getValue());
        ImageRepresentationGL::ScopedRepresentation plaque(dataContainer, p_plaqueImage.getValue());

        if (cm && tc && plaque) {
            // prepare OpenGL
            _shader->activate();
            cgt::TextureUnit ivusUnit, cmUnit, tcUnit, plaqueUnit;
            img->bind(_shader, ivusUnit, "_ivus", "_ivusParams");
            cm->bind(_shader, cmUnit, "_cm", "_cmParams");
            tc->bind(_shader, tcUnit, "_tc", "_tcParams");
            plaque->bind(_shader, plaqueUnit, "_plaque", "_plaqueParams");

            {
                cgt::Shader::IgnoreUniformLocationErrorGuard guard(_shader);
                p_predicateHistogram.getPredicateHistogram()->setupRenderShader(_shader);
            }
            
            cgt::mat4 identity = cgt::mat4::identity;

            _shader->setUniform("_texCoordsMatrix", _texCoordMatrix);
            _shader->setUniform("_modelMatrix", identity);
            _shader->setUniform("_viewMatrix", _viewMatrix);
            _shader->setUniform("_projectionMatrix", identity);
            _shader->setUniform("_useTexturing", true);
            _shader->setUniform("_useSolidColor", true);
            _shader->setUniform("_mixWithIvus", p_mixWithIvus.getValue());

            // render slice
            FramebufferActivationGuard fag(this);
            createAndAttachColorTexture();
            createAndAttachDepthTexture();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            QuadRdr.renderQuad();

            if (p_renderCrosshair.getValue())
                renderCrosshair(img);

            renderGeometry(dataContainer, img);

            _shader->deactivate();
            cgt::TextureUnit::setZeroUnit();

            dataContainer.addData(p_targetImageID.getValue(), new RenderData(_fbo));
        }
    }

    void IvusTcSliceExtractor::onHistogramHeaderChanged() {
        invalidate(INVALID_SHADER);
    }

    std::string IvusTcSliceExtractor::getGlslHeader() {
        return SliceRenderProcessor::getGlslHeader() + p_predicateHistogram.getPredicateHistogram()->getGlslHeader();
    }

}
