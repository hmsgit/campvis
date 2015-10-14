// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#include "registrationsliceview.h"

#include "cgt/glmath.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"

#include "core/classification/simpletransferfunction.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"
#include "core/datastructures/transformdata.h"
#include "core/tools/quadrenderer.h"

namespace campvis {
namespace registration {

    const std::string RegistrationSliceView::loggerCat_ = "CAMPVis.modules.vis.RegistrationSliceView";

    RegistrationSliceView::RegistrationSliceView(IVec2Property* viewportSizeProp)
        : SliceRenderProcessor(viewportSizeProp, "modules/registration/glsl/registrationsliceview.frag", "")
        , p_movingImage("MovingImage", "Moving Image", "movingImage", DataNameProperty::READ)
        , p_movingTransformationMatrix("MovingTransformationMatrix", "Moving Image Transformation Matrix", "movingImage.transformation", DataNameProperty::READ)
        , p_referenceTransferFunction("ReferenceTransferFunction", "Reference Image Transfer Function", new SimpleTransferFunction(256))
        , p_movingTransferFunction("MovingTransferFunction", "Moving Image Transfer Function", new SimpleTransferFunction(256))
    {
        addProperty(p_movingImage, INVALID_PROPERTIES | INVALID_RESULT);
        addProperty(p_movingTransformationMatrix);
        addProperty(p_referenceTransferFunction);
        addProperty(p_movingTransferFunction);

        dynamic_cast<SimpleTransferFunction*>(p_movingTransferFunction.getTF())->setRightColor(cgt::col4(192, 128, 32, 255));
    }

    RegistrationSliceView::~RegistrationSliceView() {

    }

    void RegistrationSliceView::renderImageImpl(DataContainer& dataContainer, const ImageRepresentationGL::ScopedRepresentation& refImage) {
        ImageRepresentationGL::ScopedRepresentation movImage(dataContainer, p_movingImage.getValue());
        ScopedTypedData<TransformData> movingTrafo(dataContainer, p_movingTransformationMatrix.getValue());

        if (movImage) {
            // prepare OpenGL
            _shader->activate();
            cgt::TextureUnit refImageUnit, refTfUnit, movImageUnit, movTfUnit;
            refImage->bind(_shader, refImageUnit, "_refImage", "_refImageParams");
            movImage->bind(_shader, movImageUnit, "_movImage", "_movImageParams");
            p_referenceTransferFunction.getTF()->bind(_shader, refTfUnit, "_refTf", "_refTfParams");
            p_movingTransferFunction.getTF()->bind(_shader, movTfUnit, "_movTf", "_movTfParams");

            cgt::mat4 identity = cgt::mat4::identity;
            cgt::mat4 trafoMatrix = (movingTrafo ? movingTrafo->getTransform() : cgt::mat4::identity);
            cgt::mat4 trafoMatrixInverse;
            if (! trafoMatrix.invert(trafoMatrixInverse)) {
                LERROR("Could not invert registration transformation matrix, using identity transformation!");
                trafoMatrixInverse = cgt::mat4::identity;
            }

            cgt::Bounds movingBounds = movImage->getParent()->getWorldBounds();
            cgt::vec3 halfDiagonal = movingBounds.getLLF() + (movingBounds.diagonal() / 2.f);

            _shader->setUniform("_texCoordsMatrix", _texCoordMatrix);
            _shader->setUniform("_modelMatrix", identity);
            _shader->setUniform("_viewMatrix", _viewMatrix);
            _shader->setUniform("_projectionMatrix", identity);
            _shader->setUniform("_trafoMatrix", trafoMatrixInverse);
            _shader->setUniform("_halfDiagonal", halfDiagonal);
            _shader->setUniform("_useTexturing", true);

            // render slice
            FramebufferActivationGuard fag(this);
            createAndAttachColorTexture();
            createAndAttachDepthTexture();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            QuadRdr.renderQuad();

            _shader->deactivate();
            cgt::TextureUnit::setZeroUnit();

            dataContainer.addData(p_targetImageID.getValue(), new RenderData(_fbo));
        }
    }

    void RegistrationSliceView::updateProperties(DataContainer& dataContainer) {
        ScopedTypedData<ImageData> refImage(dataContainer, p_sourceImageID.getValue());
        ScopedTypedData<ImageData> movingImage(dataContainer, p_movingImage.getValue());
        p_referenceTransferFunction.setImageHandle(refImage.getDataHandle());
        p_movingTransferFunction.setImageHandle(movingImage.getDataHandle());

        SliceRenderProcessor::updateProperties(dataContainer);
    }

}
}
