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
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/vis/glsl/slicerenderer3d.frag", "", false);
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void SliceRenderer3D::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void SliceRenderer3D::process(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_sourceImageID.getValue());

        if (img != 0) {
            if (img->getDimensionality() == 3) {
                const tgt::Camera& cam = p_camera.getValue();
                const tgt::svec3& imgSize = img->getSize();

                // Creating the slice proxy geometry works as follows:
                // Create the cube proxy geometry for the volume, then clip the cube against the slice plane.
                // The closing face is the slice proxy geometry.
                // This is probably not the fastest, but an elegant solution, which also supports arbitrary slice orientations. :)
                tgt::Bounds volumeExtent = img->getParent()->getWorldBounds();
                MeshGeometry cube = MeshGeometry::createCube(volumeExtent, tgt::Bounds(tgt::vec3(0.f), tgt::vec3(1.f)));

                tgt::vec3 normal(0.f, 0.f, 1.f);
                float p = img->getParent()->getMappingInformation().getOffset().z + (p_sliceNumber.getValue() * img->getParent()->getMappingInformation().getVoxelSize().z);
                MeshGeometry clipped = cube.clipAgainstPlane(p, normal, true);
                FaceGeometry slice = clipped.getFaces().back(); // the last face is the closing face

                FramebufferActivationGuard fag(this);
                createAndAttachColorTexture();
                createAndAttachDepthTexture();

                glEnable(GL_DEPTH_TEST);
                _shader->activate();

                _shader->setIgnoreUniformLocationError(true);
                _shader->setUniform("_viewportSizeRCP", 1.f / tgt::vec2(getEffectiveViewportSize()));
                _shader->setUniform("_projectionMatrix", cam.getProjectionMatrix());
                _shader->setUniform("_viewMatrix", cam.getViewMatrix());

                tgt::mat4 trafoMatrix = tgt::mat4::createScale(tgt::vec3(-1.f, 1.f, -1.f));
                _shader->setUniform("_modelMatrix", trafoMatrix);

                tgt::TextureUnit inputUnit, tfUnit;
                img->bind(_shader, inputUnit);
                p_transferFunction.getTF()->bind(_shader, tfUnit);

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
