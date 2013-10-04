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

#include "depthdarkening.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"


#include "core/classification/simpletransferfunction.h"

#include "core/tools/quadrenderer.h"

namespace campvis {
    const std::string DepthDarkening::loggerCat_ = "CAMPVis.modules.vis.DepthDarkening";

    DepthDarkening::DepthDarkening(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputImage("InputImage", "Input Image", "", DataNameProperty::READ)
        , p_outputImage("OutputImage", "Output Image", "dd.output", DataNameProperty::WRITE)
        , p_sigma("Sigma", "Sigma of Gaussian Filter", 2.f, 0.f, 10.f, 0.1f)
        , p_lambda("Lambda", "Strength of Depth Darkening Effect", 10.f, 0.f, 50.f, 0.1f)
        , p_useColorCoding("UseColorCoding", "Cold/Warm Color Coding", false, AbstractProcessor::INVALID_SHADER)
        , p_coldColor("ColdColor", "Cold Color (Far Objects)", tgt::vec3(0.f, 0.f, 1.f), tgt::vec3(0.f), tgt::vec3(1.f))
        , p_warmColor("WarmColor", "Warm Color (Near Objects)", tgt::vec3(1.f, 0.f, 0.f), tgt::vec3(0.f), tgt::vec3(1.f))
        , _shader(0)
    {
        addProperty(&p_inputImage);
        addProperty(&p_outputImage);
        addProperty(&p_sigma);
        addProperty(&p_lambda);
        addProperty(&p_useColorCoding);
        addProperty(&p_coldColor);
        addProperty(&p_warmColor);
    }

    DepthDarkening::~DepthDarkening() {

    }

    void DepthDarkening::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/vis/glsl/depthdarkening.frag", generateHeader(), false);
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void DepthDarkening::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void DepthDarkening::process(DataContainer& data) {
        ScopedTypedData<RenderData> inputImage(data, p_inputImage.getValue());

        if (inputImage != 0 && inputImage->hasDepthTexture()) {
            if (hasInvalidShader()) {
                _shader->setHeaders(generateHeader());
                _shader->rebuild();
                validate(INVALID_SHADER);
            }

            // TODO: const cast is ugly...
            const tgt::Texture* tex = inputImage->getDepthTexture()->getRepresentation<ImageRepresentationGL>()->getTexture();
            const_cast<tgt::Texture*>(tex)->downloadTexture();
            const float* pixels = reinterpret_cast<const float*>(tex->getPixelData());
            float curDepth = *(pixels);
            float minDepth = curDepth;
            float maxDepth = curDepth;
            size_t numPixels = inputImage->getDepthTexture()->getNumElements();
            for (size_t i = 1; i < numPixels; ++i) {
                curDepth = pixels[i];
                minDepth = std::min(minDepth, curDepth);
                maxDepth = std::max(maxDepth, curDepth);
            }

            FramebufferActivationGuard fag(this);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_ALWAYS);

            _shader->activate();
            tgt::TextureUnit colorUnit, depthUnit, pass2DepthUnit;
            inputImage->bind(_shader, colorUnit, depthUnit);
            inputImage->bindDepthTexture(_shader, pass2DepthUnit, "_depthPass2Texture", "_pass2TexParams");
            
            _shader->setUniform("_viewportSizeRCP", 1.f / tgt::vec2(getEffectiveViewportSize()));
            _shader->setUniform("_direction", tgt::vec2(1.f, 0.f));
            _shader->setUniform("_sigma", p_sigma.getValue());
            _shader->setUniform("_lambda", p_lambda.getValue());
            _shader->setUniform("_minDepth", minDepth);
            _shader->setUniform("_maxDepth", maxDepth);
            if (p_useColorCoding.getValue()) {
                _shader->setUniform("_coldColor", p_coldColor.getValue());
                _shader->setUniform("_warmColor", p_warmColor.getValue());
            }

            createAndAttachDepthTexture();
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            QuadRdr.renderQuad();

            RenderData tempTarget(_fbo);
            _fbo->detachAll();
            createAndAttachColorTexture();
            createAndAttachDepthTexture();

            inputImage->bind(_shader, colorUnit, depthUnit);
            tempTarget.bindDepthTexture(_shader, pass2DepthUnit, "_depthPass2Texture", "_pass2TexParams");
            _shader->setUniform("_direction", tgt::vec2(0.f, 1.f));
            
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            QuadRdr.renderQuad();

            _shader->deactivate();
            tgt::TextureUnit::setZeroUnit();
            glDepthFunc(GL_LESS);
            glDisable(GL_DEPTH_TEST);
            LGL_ERROR;

            data.addData(p_outputImage.getValue(), new RenderData(_fbo));
            p_outputImage.issueWrite();
        }
        else {
            LERROR("No suitable input image found.");
        }

        validate(INVALID_RESULT);
    }

    std::string DepthDarkening::generateHeader() const {
        if (p_useColorCoding.getValue())
            return "#define USE_COLORCODING\n";
        else
            return "";
    }

}

