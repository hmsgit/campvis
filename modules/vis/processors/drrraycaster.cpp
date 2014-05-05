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

#include "drrraycaster.h"

#include "tgt/shadermanager.h"
#include "core/tools/quadrenderer.h"
#include "core/datastructures/renderdata.h"

namespace campvis {
    const std::string DRRRaycaster::loggerCat_ = "CAMPVis.modules.vis.DRRRaycaster";

    DRRRaycaster::DRRRaycaster(IVec2Property* viewportSizeProp)
        : RaycastingProcessor(viewportSizeProp, "modules/vis/glsl/drrraycaster.frag", false)
        , p_shift("shift", "Normalization Shift", 0.f, -10.f, 10.f, 0.1f)
        , p_scale("scale", "Normalization Scale", 1.f, 0.f, 1000.f, 0.1f)
        , p_invertMapping("InvertMapping", "Invert Mapping", false)
    {
        addProperty(p_shift);
        addProperty(p_scale);
        addProperty(p_invertMapping, INVALID_RESULT | INVALID_SHADER);
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
