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

#include "slicerenderer3d.h"
#include "tgt/bounds.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"

#include "core/datastructures/meshgeometry.h"
#include "core/datastructures/facegeometry.h"
#include "core/datastructures/geometrydatafactory.h"

#include "core/classification/simpletransferfunction.h"

#include "core/tools/quadrenderer.h"

namespace campvis {
    const std::string SliceRenderer3D::loggerCat_ = "CAMPVis.modules.vis.SliceRenderer3D";

    SliceRenderer3D::SliceRenderer3D(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_sourceImageID("sourceImageID", "Input Image", "", DataNameProperty::READ, AbstractProcessor::INVALID_RESULT | AbstractProcessor::INVALID_PROPERTIES)
        , p_targetImageID("targetImageID", "Output Image", "", DataNameProperty::WRITE)
        , p_camera("Camera", "Camera")
        , p_sliceNumber("sliceNumber", "Slice Number", 0, 0, 0)
        , p_transferFunction("transferFunction", "Transfer Function", new SimpleTransferFunction(256))
        , _shader(0)
    {
        addProperty(&p_sourceImageID);
        addProperty(&p_targetImageID);
        addProperty(&p_camera);
        addProperty(&p_sliceNumber);
        addProperty(&p_transferFunction);
    }

    SliceRenderer3D::~SliceRenderer3D() {

    }

    void SliceRenderer3D::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/vis/glsl/slicerenderer3d.frag", "");
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void SliceRenderer3D::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void SliceRenderer3D::updateResult(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_sourceImageID.getValue());

        if (img != 0) {
            if (img->getDimensionality() == 3) {
                const tgt::Camera& cam = p_camera.getValue();

                // Creating the slice proxy geometry works as follows:
                // Create the cube proxy geometry for the volume, then clip the cube against the slice plane.
                // The closing face is the slice proxy geometry.
                // This is probably not the fastest, but an elegant solution, which also supports arbitrary slice orientations. :)
                tgt::Bounds volumeExtent = img->getParent()->getWorldBounds();
                MeshGeometry* cube = GeometryDataFactory::createCube(volumeExtent, tgt::Bounds(tgt::vec3(0.f), tgt::vec3(1.f)));

                tgt::vec3 normal(0.f, 0.f, 1.f);
                float p = img->getParent()->getMappingInformation().getOffset().z + (p_sliceNumber.getValue() * img->getParent()->getMappingInformation().getVoxelSize().z);
                MeshGeometry clipped = cube->clipAgainstPlane(p, normal, true);
                const FaceGeometry& slice = clipped.getFaces().back(); // the last face is the closing face
                delete cube;

                glEnable(GL_DEPTH_TEST);
                _shader->activate();

                _shader->setIgnoreUniformLocationError(true);
                _shader->setUniform("_viewportSizeRCP", 1.f / tgt::vec2(getEffectiveViewportSize()));
                _shader->setUniform("_projectionMatrix", cam.getProjectionMatrix());
                _shader->setUniform("_viewMatrix", cam.getViewMatrix());

                tgt::TextureUnit inputUnit, tfUnit;
                img->bind(_shader, inputUnit);
                p_transferFunction.getTF()->bind(_shader, tfUnit);

                FramebufferActivationGuard fag(this);
                createAndAttachColorTexture();
                createAndAttachDepthTexture();
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                slice.render(GL_POLYGON);

                _shader->deactivate();
                tgt::TextureUnit::setZeroUnit();
                glDisable(GL_DEPTH_TEST);

                data.addData(p_targetImageID.getValue(), new RenderData(_fbo));
            }
            else {
                LERROR("Input image must have dimensionality of 3.");
            }
        }
        else {
            LERROR("No suitable input image found.");
        }

        validate(INVALID_RESULT);
    }

    void SliceRenderer3D::updateProperties(DataContainer& dc) {
        ScopedTypedData<ImageData> img(dc, p_sourceImageID.getValue());

        if (img != 0) {
            const tgt::svec3& imgSize = img->getSize();
            if (p_sliceNumber.getMaxValue() != static_cast<int>(imgSize.z) - 1){
                p_sliceNumber.setMaxValue(static_cast<int>(imgSize.z) - 1);
            }
        }
                        
        validate(AbstractProcessor::INVALID_PROPERTIES);
    }

}
