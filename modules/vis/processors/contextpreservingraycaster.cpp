// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "contextpreservingraycaster.h"

#include "core/tools/quadrenderer.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorshading.h"

#include <tbb/tbb.h>

namespace campvis {
    const std::string ContextPreservingRaycaster::loggerCat_ = "CAMPVis.modules.vis.ContextPreservingRaycaster";

    ContextPreservingRaycaster::ContextPreservingRaycaster(IVec2Property* viewportSizeProp)
        : RaycastingProcessor(viewportSizeProp, "modules/vis/glsl/contextpreservingraycaster.frag", true)
        , p_kappaS("KappaS", "k_s (from the paper)", .5f, .0f, 1.f, .1f, 1)
        , p_kappaT("KappaT", "k_t (from the paper)", 3.f, 0.f, 10.f, .1f, 1)
    {
        addDecorator(new ProcessorDecoratorShading());

        addProperty(&p_kappaS);
        addProperty(&p_kappaT);

        decoratePropertyCollection(this);
    }

    ContextPreservingRaycaster::~ContextPreservingRaycaster() {
    }

    void ContextPreservingRaycaster::init() {
        RaycastingProcessor::init();
    }

    void ContextPreservingRaycaster::deinit() {
        RaycastingProcessor::deinit();
    }

    void ContextPreservingRaycaster::processImpl(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image) {
        _shader->setUniform("_kappaS", p_kappaS.getValue());
        _shader->setUniform("_kappaT", p_kappaT.getValue());

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

        data.addData(p_targetImageID.getValue(), new RenderData(_fbo));
    }

    std::string ContextPreservingRaycaster::generateHeader() const {
        std::string toReturn = RaycastingProcessor::generateHeader();
        return toReturn;
    }

    void ContextPreservingRaycaster::updateProperties(DataContainer& dataContainer) {
        RaycastingProcessor::updateProperties(dataContainer);
    }

}
