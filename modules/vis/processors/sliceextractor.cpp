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

#include "sliceextractor.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"
#include "core/classification/simpletransferfunction.h"

#include "core/tools/quadrenderer.h"

namespace campvis {
    const std::string SliceExtractor::loggerCat_ = "CAMPVis.modules.vis.SliceExtractor";

    SliceExtractor::SliceExtractor(IVec2Property* viewportSizeProp)
        : SliceRenderProcessor(viewportSizeProp, "modules/vis/glsl/sliceextractor.frag", "")
        , p_transferFunction("TransferFunction", "Transfer Function", new SimpleTransferFunction(256))
    {
        addProperty(p_transferFunction);
    }

    SliceExtractor::~SliceExtractor() {

    }

    void SliceExtractor::renderImageImpl(DataContainer& dataContainer, const ImageRepresentationGL::ScopedRepresentation& img) {

        // prepare OpenGL
        _shader->activate();
        tgt::TextureUnit inputUnit, tfUnit;
        img->bind(_shader, inputUnit);
        p_transferFunction.getTF()->bind(_shader, tfUnit);

        tgt::mat4 identity = tgt::mat4::identity;

        _shader->setUniform("_texCoordsMatrix", _texCoordMatrix);
        _shader->setUniform("_modelMatrix", identity);
        _shader->setUniform("_viewMatrix", _viewMatrix);
        _shader->setUniform("_projectionMatrix", identity);
        _shader->setUniform("_useTexturing", true);
        _shader->setUniform("_useSolidColor", true);

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
        tgt::TextureUnit::setZeroUnit();

        dataContainer.addData(p_targetImageID.getValue(), new RenderData(_fbo));
    }

    void SliceExtractor::updateProperties(DataContainer& dc) {
        ScopedTypedData<ImageData> img(dc, p_sourceImageID.getValue());
        p_transferFunction.setImageHandle(img.getDataHandle());

        SliceRenderProcessor::updateProperties(dc);
    }

}
