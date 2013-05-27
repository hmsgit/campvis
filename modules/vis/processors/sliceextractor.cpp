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

#include "sliceextractor.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/imagerepresentationrendertarget.h"
#include "core/pipeline/processordecoratorbackground.h"

#include "core/classification/simpletransferfunction.h"

#include "core/tools/quadrenderer.h"

namespace campvis {
    static const GenericOption<SliceExtractor::SliceOrientation> compositingOptions[3] = {
        GenericOption<SliceExtractor::SliceOrientation>("z", "XY Plane", SliceExtractor::XY_PLANE),
        GenericOption<SliceExtractor::SliceOrientation>("y", "XZ Plane", SliceExtractor::XZ_PLANE),
        GenericOption<SliceExtractor::SliceOrientation>("x", "YZ Plane", SliceExtractor::YZ_PLANE)
    };

    const std::string SliceExtractor::loggerCat_ = "CAMPVis.modules.vis.SliceExtractor";

    SliceExtractor::SliceExtractor(IVec2Property& canvasSize)
        : VisualizationProcessor(canvasSize)
        , p_sourceImageID("sourceImageID", "Input Image", "", DataNameProperty::READ, AbstractProcessor::INVALID_PROPERTIES)
        , p_targetImageID("targetImageID", "Output Image", "", DataNameProperty::WRITE)
        , p_sliceOrientation("SliceOrientation", "Slice Orientation", compositingOptions, 3)
        , p_xSliceNumber("XSliceNumber", "X Slice Number", 0, 0, 0)
        , p_ySliceNumber("YSliceNumber", "Y Slice Number", 0, 0, 0)
        , p_zSliceNumber("ZSliceNumber", "Z Slice Number", 0, 0, 0)
        , p_transferFunction("transferFunction", "Transfer Function", new SimpleTransferFunction(256))
        , _shader(0)
    {
        addProperty(&p_sourceImageID);
        addProperty(&p_targetImageID);
        addProperty(&p_sliceOrientation);
        addProperty(&p_xSliceNumber);
        addProperty(&p_ySliceNumber);
        addProperty(&p_zSliceNumber);
        addProperty(&p_transferFunction);

        //addDecorator(new ProcessorDecoratorBackground());
        decoratePropertyCollection(this);
    }

    SliceExtractor::~SliceExtractor() {

    }

    void SliceExtractor::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/vis/glsl/sliceextractor.frag", "", false);
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void SliceExtractor::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void SliceExtractor::process(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_sourceImageID.getValue());

        if (img != 0) {
            if (img->getDimensionality() == 3) {
                if (hasInvalidProperties()) {
                    // source DataHandle has changed
                    updateProperties(img.getDataHandle());
                    validate(AbstractProcessor::INVALID_PROPERTIES);
                }

                const tgt::svec3& imgSize = img->getSize();
                std::pair<ImageData*, ImageRepresentationRenderTarget*> rt = ImageRepresentationRenderTarget::createWithImageData(_renderTargetSize.getValue());

                tgt::mat4 texCoordsMatrix = tgt::mat4::zero;
                switch (p_sliceOrientation.getValue()) {
                case XY_PLANE:
                    texCoordsMatrix = tgt::mat4::identity;
                    texCoordsMatrix.t23 = (static_cast<float>(p_zSliceNumber.getValue() + 0.5f) / static_cast<float>(imgSize.z));
                    break;

                case XZ_PLANE:
                    texCoordsMatrix.t00 = 1.f;   // setup a permutation matrix, swapping z- and y-
                    texCoordsMatrix.t12 = 1.f;   // components on vectors being multiplied with it
                    texCoordsMatrix.t21 = 1.f;
                    texCoordsMatrix.t33 = 1.f;
                    texCoordsMatrix.t13 = (static_cast<float>(p_ySliceNumber.getValue() + 0.5f) / static_cast<float>(imgSize.y));
                    break;

                case YZ_PLANE:
                    texCoordsMatrix.t01 = 1.f;   // setup a permutation matrix, swapping x-, y- and z-
                    texCoordsMatrix.t12 = 1.f;   // components on vectors being multiplied with it
                    texCoordsMatrix.t20 = 1.f;
                    texCoordsMatrix.t33 = 1.f;
                    texCoordsMatrix.t13 = (static_cast<float>(p_xSliceNumber.getValue() + 0.5f) / static_cast<float>(imgSize.x));
                    break;
                }

                _shader->activate();
                decorateRenderProlog(data, _shader);
                tgt::TextureUnit inputUnit, tfUnit;
                img->bind(_shader, inputUnit);
                p_transferFunction.getTF()->bind(_shader, tfUnit);
                _shader->setUniform("_texCoordsMatrix", texCoordsMatrix);

                rt.second->activate();
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                QuadRdr.renderQuad();
                rt.second->deactivate();

                decorateRenderEpilog(_shader);
                _shader->deactivate();
                tgt::TextureUnit::setZeroUnit();

                data.addData(p_targetImageID.getValue(), rt.first);
                p_targetImageID.issueWrite();
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

    void SliceExtractor::updateProperties(DataHandle img) {
        p_transferFunction.getTF()->setImageHandle(img);
        const tgt::svec3& imgSize = static_cast<const ImageData*>(img.getData())->getSize();
        if (p_xSliceNumber.getMaxValue() != imgSize.x - 1){
            p_xSliceNumber.setMaxValue(static_cast<int>(imgSize.x) - 1);
        }
        if (p_ySliceNumber.getMaxValue() != imgSize.y - 1){
            p_ySliceNumber.setMaxValue(static_cast<int>(imgSize.y) - 1);
        }
        if (p_zSliceNumber.getMaxValue() != imgSize.z - 1){
            p_zSliceNumber.setMaxValue(static_cast<int>(imgSize.z) - 1);
        }
    }

}
