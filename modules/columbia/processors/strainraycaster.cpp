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

#include "strainraycaster.h"

#include "core/tools/quadrenderer.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorshading.h"

namespace campvis {
    const std::string StrainRaycaster::loggerCat_ = "CAMPVis.modules.vis.StrainRaycaster";

    StrainRaycaster::StrainRaycaster(IVec2Property& canvasSize)
        : RaycastingProcessor(canvasSize, "modules/columbia/glsl/strainraycaster.frag", true)
        , p_targetImageID("targetImageID", "Output Image", "", DataNameProperty::WRITE)
        , p_enableShadowing("EnableShadowing", "Enable Hard Shadows", false, AbstractProcessor::INVALID_SHADER)
        , p_shadowIntensity("ShadowIntensity", "Shadow Intensity", .5f, .0f, 1.f, 0.01f)
        , p_enableAdaptiveStepsize("EnableAdaptiveStepSize", "Enable Adaptive Step Size", true, AbstractProcessor::INVALID_SHADER)
    {
        addDecorator(new ProcessorDecoratorShading());

        addProperty(&p_targetImageID);
        addProperty(&p_enableShadowing);
        addProperty(&p_shadowIntensity);
        addProperty(&p_enableAdaptiveStepsize);
        decoratePropertyCollection(this);
    }

    StrainRaycaster::~StrainRaycaster() {

    }

    void StrainRaycaster::processImpl(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image) {
        if (image.getImageData()->getNumChannels() == 3 || image.getImageData()->getNumChannels() == 4) {
            FramebufferActivationGuard fag(this);
            createAndAttachColorTexture();
            createAndAttachDepthTexture();

            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            QuadRdr.renderQuad();
            LGL_ERROR;

            data.addData(p_targetImageID.getValue(), new RenderData(_fbo));
            p_targetImageID.issueWrite();
        }
        else {
            LERROR("Wrong Number of Channels in Input Volume.");
        }
    }

    std::string StrainRaycaster::generateHeader() const {
        std::string toReturn = RaycastingProcessor::generateHeader();
        if (p_enableShadowing.getValue())
            toReturn += "#define ENABLE_SHADOWING\n";
        if (p_enableAdaptiveStepsize.getValue())
            toReturn += "#define ENABLE_ADAPTIVE_STEPSIZE\n";
        return toReturn;
    }

}
