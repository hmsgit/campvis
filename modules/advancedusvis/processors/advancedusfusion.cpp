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

#include "advancedusfusion.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorbackground.h"

#include "core/classification/simpletransferfunction.h"
#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

#include "core/tools/quadrenderer.h"

namespace campvis {
    const std::string AdvancedUsFusion::loggerCat_ = "CAMPVis.modules.vis.AdvancedUsFusion";

    GenericOption<std::string> viewOptions[12] = {
        GenericOption<std::string>("us", "Ultrasound Only"),
        GenericOption<std::string>("smoothed", "Smoothed US Only"),
        GenericOption<std::string>("cm", "Confidence Map US Only"),
        GenericOption<std::string>("mappingSaturationHSV", "Mapping Uncertainty to Saturation (HSV)"),
        GenericOption<std::string>("mappingSaturationHSL", "Mapping Uncertainty to Saturation (HSL)"),
        GenericOption<std::string>("mappingSaturationTSL", "Mapping Uncertainty to Saturation (TSL)"),
        GenericOption<std::string>("mappingChromacityHCL", "Mapping Uncertainty to Chromacity (HCL)"),
        GenericOption<std::string>("mappingChromacityHCY", "Mapping Uncertainty to Chromacity (HCY)"),
        GenericOption<std::string>("mappingLAB", "Mapping Uncertainty L*a*b*"),
        GenericOption<std::string>("mappingHunterLAB", "Mapping Uncertainty Hunter L*a*b*"),
        GenericOption<std::string>("mappingSharpness", "Mapping Uncertainty to Sharpness"),
        GenericOption<std::string>("pixelate", "Pixelate (Experimental)")
    };

    AdvancedUsFusion::AdvancedUsFusion(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_usImageId("UsImageId", "Ultrasound Input Image", "", DataNameProperty::READ, AbstractProcessor::INVALID_PROPERTIES | AbstractProcessor::INVALID_RESULT)
        , p_blurredImageId("BlurredImageId", "Blurred Ultrasound Image", "", DataNameProperty::READ)
        , p_gradientImageID("GradientImageId", "Gradient Input Image", "", DataNameProperty::READ)
        , p_confidenceImageID("ConfidenceImageId", "Confidence Map Input", "", DataNameProperty::READ)
        , p_targetImageID("targetImageID", "Output Image", "", DataNameProperty::WRITE)
        , p_sliceNumber("sliceNumber", "Slice Number", 0, 0, 0)
        , p_transferFunction("transferFunction", "Transfer Function", new SimpleTransferFunction(256))
        , p_confidenceTF("ConfidenceTF", "Confidence to Uncertainty TF", new Geometry1DTransferFunction(256))
        , p_view("View", "Image to Render", viewOptions, 12)
        , p_confidenceScaling("ConfidenceScaling", "Confidence Scaling", 1.f, .001f, 1000.f, 0.1f)
        , p_blurredScaling("BlurredScaling", "Blurred Scaling", 1.f, .001f, 1000.f, 0.1f)
        , p_hue("Hue", "Hue for Uncertainty Mapping", .15f, 0.f, 1.f)
        , p_use3DTexture("Use3DTexture", "Use 3D Texture", false)
        , _shader(0)
    {
        addProperty(&p_usImageId);
        addProperty(&p_blurredImageId);
        addProperty(&p_gradientImageID);
        addProperty(&p_confidenceImageID);
        addProperty(&p_blurredScaling);
        addProperty(&p_targetImageID);
        addProperty(&p_sliceNumber);
        addProperty(&p_transferFunction);
        addProperty(&p_confidenceTF);
        addProperty(&p_view);
        addProperty(&p_confidenceScaling);
        addProperty(&p_hue);

        Geometry1DTransferFunction* tf = static_cast<Geometry1DTransferFunction*>(p_confidenceTF.getTF());
        tf->addGeometry(TFGeometry1D::createQuad(tgt::vec2(0.f, 1.f), tgt::col4(0, 0, 0, 96), tgt::col4(0, 0, 0, 0)));

        decoratePropertyCollection(this);
    }

    AdvancedUsFusion::~AdvancedUsFusion() {

    }

    void AdvancedUsFusion::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/advancedusvis/glsl/advancedusfusion.frag", generateHeader(), false);
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void AdvancedUsFusion::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void AdvancedUsFusion::process(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_usImageId.getValue());
        ImageRepresentationGL::ScopedRepresentation blurred(data, p_blurredImageId.getValue());
        ImageRepresentationGL::ScopedRepresentation confidence(data, p_confidenceImageID.getValue());

        if (img != 0 && blurred != 0 && confidence != 0) {
            if (img->getDimensionality() >= 2) {
                FramebufferActivationGuard fag(this);
                createAndAttachColorTexture();
                createAndAttachDepthTexture();

                _shader->activate();
                decorateRenderProlog(data, _shader);
                if (p_use3DTexture.getValue())
                    _shader->setUniform("_sliceNumber", p_sliceNumber.getValue());
                _shader->setUniform("_viewIndex", p_view.getValue());
                _shader->setUniform("_confidenceScaling", p_confidenceScaling.getValue());
                _shader->setUniform("_hue", p_hue.getValue());
                _shader->setUniform("_blurredScale", 1.f / p_blurredScaling.getValue());
                
                tgt::TextureUnit usUnit, blurredUnit, confidenceUnit, tfUnit, tf2Unit;
                img->bind(_shader, usUnit, "_usImage", "_usTextureParams");
                blurred->bind(_shader, blurredUnit, "_blurredImage", "_blurredTextureParams");
                confidence->bind(_shader, confidenceUnit, "_confidenceMap", "_confidenceTextureParams");
                p_transferFunction.getTF()->bind(_shader, tfUnit);
                p_confidenceTF.getTF()->bind(_shader, tf2Unit, "_confidenceTF", "_confidenceTFParams");

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                QuadRdr.renderQuad();

                decorateRenderEpilog(_shader);
                _shader->deactivate();
                tgt::TextureUnit::setZeroUnit();

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

    void AdvancedUsFusion::updateProperties(DataContainer dc) {
        ScopedTypedData<ImageData> img(dc, p_usImageId.getValue());

        p_transferFunction.getTF()->setImageHandle(img.getDataHandle());
        const tgt::svec3& imgSize = img->getSize();
        if (p_sliceNumber.getMaxValue() != imgSize.z - 1){
            p_sliceNumber.setMaxValue(static_cast<int>(imgSize.z) - 1);
        }
        p_use3DTexture.setValue(img->getDimensionality() == 3);

        validate(AbstractProcessor::INVALID_PROPERTIES);
    }

    std::string AdvancedUsFusion::generateHeader() const {
        std::string toReturn = getDecoratedHeader();
        if (p_use3DTexture.getValue())
            toReturn += "#define USE_3D_TEX 1\n";
        return toReturn;
    }

}
