// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#include "simpleraycaster.h"

#include "core/tools/quadrenderer.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorshading.h"

namespace campvis {
    const std::string SimpleRaycaster::loggerCat_ = "CAMPVis.modules.vis.SimpleRaycaster";

    SimpleRaycaster::SimpleRaycaster(IVec2Property* viewportSizeProp)
        : RaycastingProcessor(viewportSizeProp, "modules/vis/glsl/simpleraycaster.frag", true)
        , p_targetImageID("targetImageID", "Output Image", "", DataNameProperty::WRITE)
        , p_enableShadowing("EnableShadowing", "Enable Hard Shadows (Expensive!)", false, AbstractProcessor::INVALID_RESULT | AbstractProcessor::INVALID_SHADER | AbstractProcessor::INVALID_PROPERTIES)
        , p_shadowIntensity("ShadowIntensity", "Shadow Intensity", .5f, .0f, 1.f)
        , p_enableAdaptiveStepsize("EnableAdaptiveStepSize", "Enable Adaptive Step Size", true, AbstractProcessor::INVALID_RESULT | AbstractProcessor::INVALID_SHADER)
    {
        addDecorator(new ProcessorDecoratorShading());

        addProperty(&p_targetImageID);
        addProperty(&p_enableAdaptiveStepsize);

        addProperty(&p_enableShadowing);
        addProperty(&p_shadowIntensity);
        p_shadowIntensity.setVisible(false);

        decoratePropertyCollection(this);
    }

    SimpleRaycaster::~SimpleRaycaster() {

    }

    void SimpleRaycaster::processImpl(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image) {
        FramebufferActivationGuard fag(this);
        createAndAttachTexture(GL_RGBA8);
        createAndAttachTexture(GL_RGBA32F);
        createAndAttachTexture(GL_RGBA32F);
        createAndAttachDepthTexture();

        static const GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 , GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, buffers);

        if (p_enableShadowing.getValue())
            _shader->setUniform("_shadowIntensity", p_shadowIntensity.getValue());

        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        QuadRdr.renderQuad();
        glDisable(GL_DEPTH_TEST);
        LGL_ERROR;

        data.addData(p_targetImageID.getValue(), new RenderData(_fbo));
    }

    std::string SimpleRaycaster::generateHeader() const {
        std::string toReturn = RaycastingProcessor::generateHeader();
        if (p_enableShadowing.getValue())
            toReturn += "#define ENABLE_SHADOWING\n";
        if (p_enableAdaptiveStepsize.getValue())
            toReturn += "#define ENABLE_ADAPTIVE_STEPSIZE\n";
        return toReturn;
    }

    void SimpleRaycaster::updateProperties() {
        p_shadowIntensity.setVisible(p_enableShadowing.getValue());
        validate(AbstractProcessor::INVALID_PROPERTIES);
    }

}
