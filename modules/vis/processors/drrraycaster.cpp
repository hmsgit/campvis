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

#include "drrraycaster.h"

#include "tgt/shadermanager.h"
#include "core/tools/quadrenderer.h"
#include "core/datastructures/renderdata.h"

namespace campvis {
    const std::string DRRRaycaster::loggerCat_ = "CAMPVis.modules.vis.DRRRaycaster";

    DRRRaycaster::DRRRaycaster(IVec2Property& canvasSize)
        : RaycastingProcessor(canvasSize, "modules/vis/glsl/drrraycaster.frag", false)
        , p_targetImageID("targetImageID", "Output Image", "", DataNameProperty::WRITE)
        , p_shift("shift", "Normalization Shift", 0.f, -10.f, 10.f, 0.1f)
        , p_scale("scale", "Normalization Scale", 1.f, 0.f, 1000.f, 0.1f)
        , p_invertMapping("invertMapping", "Invert Mapping", false, AbstractProcessor::INVALID_RESULT | AbstractProcessor::INVALID_SHADER)
    {
        addProperty(&p_targetImageID);
        addProperty(&p_shift);
        addProperty(&p_scale);
        addProperty(&p_invertMapping);
    }

    DRRRaycaster::~DRRRaycaster() {

    }

    void DRRRaycaster::processImpl(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image) {
        FramebufferActivationGuard fag(this);
        createAndAttachColorTexture();
        createAndAttachDepthTexture();

        _shader->setUniform("_shift", p_shift.getValue());
        _shader->setUniform("_scale", p_scale.getValue());

        if (p_invertMapping.getValue())
            glClearColor(0.f, 0.f, 0.f, 1.f);
        else
            glClearColor(1.f, 1.f, 1.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        QuadRdr.renderQuad();
        LGL_ERROR;

        data.addData(p_targetImageID.getValue(), new RenderData(_fbo));
        p_targetImageID.issueWrite();
    }

    std::string DRRRaycaster::generateHeader() const {
        std::string toReturn;

        if (p_invertMapping.getValue())
            toReturn += "#define DRR_INVERT 1\n";
        //         if (depthMapping_.get())
        //             header +="#define DEPTH_MAPPING 1\n";

        return toReturn;
    }
}
