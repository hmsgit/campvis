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

#include "strainraycaster.h"

#include "core/tools/quadrenderer.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorshading.h"

namespace campvis {
    const std::string StrainRaycaster::loggerCat_ = "CAMPVis.modules.vis.StrainRaycaster";

    StrainRaycaster::StrainRaycaster(IVec2Property* viewportSizeProp)
        : RaycastingProcessor(viewportSizeProp, "modules/columbia/glsl/strainraycaster.frag", true)
        , p_targetImageID("targetImageID", "Output Image", "", DataNameProperty::WRITE)
        , p_enableShadowing("EnableShadowing", "Enable Hard Shadows", false)
        , p_shadowIntensity("ShadowIntensity", "Shadow Intensity", .5f, .0f, 1.f)
        , p_enableAdaptiveStepsize("EnableAdaptiveStepSize", "Enable Adaptive Step Size", true)
    {
        addDecorator(new ProcessorDecoratorShading());

        addProperty(p_targetImageID);
        addProperty(p_enableShadowing, INVALID_SHADER | INVALID_RESULT);
        addProperty(p_shadowIntensity);
        addProperty(p_enableAdaptiveStepsize, INVALID_SHADER | INVALID_RESULT);
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
