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

#include "advancedusfusion.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"

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

    GenericOption<std::string> viewOptions[14] = {
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
        GenericOption<std::string>("pixelate", "Pixelate (Experimental)"),
        GenericOption<std::string>("colorOverlay", "Color Overlay"),
        GenericOption<std::string>("mappingHybrid", "Hybrid Mapping to Chroma and Sharpness")
    };

    AdvancedUsFusion::AdvancedUsFusion(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_usImageId("UsImageId", "Ultrasound Input Image", "", DataNameProperty::READ)
        , p_blurredImageId("BlurredImageId", "Blurred Ultrasound Image", "", DataNameProperty::READ)
        , p_gradientImageID("GradientImageId", "Gradient Input Image", "", DataNameProperty::READ)
        , p_confidenceImageID("ConfidenceImageId", "Confidence Map Input", "", DataNameProperty::READ)
        , p_targetImageID("targetImageID", "Output Image", "", DataNameProperty::WRITE)
        , p_renderToTexture("RenderToTexture", "Render to an OpenGL Texture", false)
        , p_sliceNumber("sliceNumber", "Slice Number", 0, 0, 0)
        , p_transferFunction("transferFunction", "Transfer Function", new SimpleTransferFunction(256))
        , p_confidenceTF("ConfidenceTF", "Confidence to Uncertainty TF", new Geometry1DTransferFunction(256))
        , p_view("View", "Image to Render", viewOptions, 14)
        , p_blurredScaling("BlurredScaling", "Blurred Scaling", 1.f, .001f, 1000.f, 0.1f)
        , p_confidenceScaling("ConfidenceScaling", "Confidence Scaling", 1.f, .001f, 1000.f, 0.1f)
        , p_hue("Hue", "Hue for Uncertainty Mapping", .15f, 0.f, 1.f)
        , p_mixFactor("MixFactor", "Mix Factor", .5f, 0.f, 1.f, .1f, 1)
        , p_use3DTexture("Use3DTexture", "Use 3D Texture", false)
        , _shader(0)
    {
        addProperty(p_usImageId, INVALID_PROPERTIES | INVALID_RESULT);
        addProperty(p_blurredImageId);
        addProperty(p_gradientImageID);
        addProperty(p_confidenceImageID);
        addProperty(p_blurredScaling);
        addProperty(p_renderToTexture);
        addProperty(p_targetImageID);
        addProperty(p_sliceNumber);
        addProperty(p_transferFunction);
        addProperty(p_confidenceTF);
        addProperty(p_view);
        addProperty(p_confidenceScaling);
        addProperty(p_hue);
        addProperty(p_mixFactor);
        p_mixFactor.setVisible(false);

        Geometry1DTransferFunction* tf = static_cast<Geometry1DTransferFunction*>(p_confidenceTF.getTF());
        tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(0.f, 1.f), cgt::col4(0, 0, 0, 96), cgt::col4(0, 0, 0, 0)));

        decoratePropertyCollection(this);
    }

    AdvancedUsFusion::~AdvancedUsFusion() {

    }

    void AdvancedUsFusion::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/advancedusvis/glsl/advancedusfusion.frag", generateHeader());
    }

    void AdvancedUsFusion::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void AdvancedUsFusion::updateResult(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_usImageId.getValue());
        ImageRepresentationGL::ScopedRepresentation blurred(data, p_blurredImageId.getValue());
        ImageRepresentationGL::ScopedRepresentation confidence(data, p_confidenceImageID.getValue());

        if (img != 0 && blurred != 0 && confidence != 0) {
            if (img->getDimensionality() >= 2) {
                _shader->activate();
                decorateRenderProlog(data, _shader);
                if (p_use3DTexture.getValue())
                    _shader->setUniform("_sliceNumber", p_sliceNumber.getValue());
                _shader->setUniform("_viewIndex", p_view.getValue());
                _shader->setUniform("_confidenceScaling", p_confidenceScaling.getValue());
                _shader->setUniform("_hue", p_hue.getValue());
                _shader->setUniform("_blurredScale", 1.f / p_blurredScaling.getValue());
                _shader->setUniform("_mixFactor", p_mixFactor.getValue());
                
                cgt::TextureUnit usUnit, blurredUnit, confidenceUnit, tfUnit, tf2Unit;
                img->bind(_shader, usUnit, "_usImage", "_usTextureParams");
                blurred->bind(_shader, blurredUnit, "_blurredImage", "_blurredTextureParams");
                confidence->bind(_shader, confidenceUnit, "_confidenceMap", "_confidenceTextureParams");
                p_transferFunction.getTF()->bind(_shader, tfUnit);
                p_confidenceTF.getTF()->bind(_shader, tf2Unit, "_confidenceTF", "_confidenceTFParams");

                if (p_renderToTexture.getValue() == true) {
                    cgt::vec3 size = img->getSize();
                    cgt::Texture* resultTexture = new cgt::Texture(GL_TEXTURE_2D, size, GL_RGB8, cgt::Texture::LINEAR);

                    _fbo->activate();
                    glViewport(0, 0, static_cast<GLsizei>(size.x), static_cast<GLsizei>(size.y));
                    _fbo->attachTexture(resultTexture, GL_COLOR_ATTACHMENT0, 0, 0);
                    LGL_ERROR;

                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    QuadRdr.renderQuad();

                    _fbo->detachAll();
                    _fbo->deactivate();
                    _shader->deactivate();
                    ImageData* id = new ImageData(img->getParent()->getDimensionality(), size, 3);
                    ImageRepresentationGL::create(id, resultTexture);
                    id->setMappingInformation(img->getParent()->getMappingInformation());
                    cgt::TextureUnit::setZeroUnit();

                    data.addData(p_targetImageID.getValue(), id);
                }
                else {
                    FramebufferActivationGuard fag(this);
                    createAndAttachColorTexture();
                    createAndAttachDepthTexture();

                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                    QuadRdr.renderQuad();

                    decorateRenderEpilog(_shader);
                    _shader->deactivate();
                    cgt::TextureUnit::setZeroUnit();

                    data.addData(p_targetImageID.getValue(), new RenderData(_fbo));
                }
            }
            else {
                LERROR("Input image must have dimensionality of 3.");
            }
        }
        else {
            LDEBUG("No suitable input image found.");
        }
    }

    void AdvancedUsFusion::updateProperties(DataContainer& dc) {
        ScopedTypedData<ImageData> img(dc, p_usImageId.getValue());

        p_transferFunction.setImageHandle(img.getDataHandle());
        if (img != nullptr) {
            const cgt::svec3& imgSize = img->getSize();
            if (static_cast<cgt::svec3::ElemType> (p_sliceNumber.getMaxValue()) != imgSize.z - 1){
                p_sliceNumber.setMaxValue(static_cast<int>(imgSize.z) - 1);
            }
            p_use3DTexture.setValue(img->getDimensionality() == 3);
        }

        p_mixFactor.setVisible(p_view.getOptionId() == "mappingHybrid");
    }

    std::string AdvancedUsFusion::generateHeader() const {
        std::string toReturn = getDecoratedHeader();
        if (p_use3DTexture.getValue())
            toReturn += "#define USE_3D_TEX 1\n";
        return toReturn;
    }

}
