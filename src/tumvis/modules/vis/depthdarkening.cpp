// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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
#include "core/datastructures/imagedatagl.h"
#include "core/datastructures/imagedatarendertarget.h"
#include "core/datastructures/imagedataconverter.h"

#include "core/classification/simpletransferfunction.h"

#include "core/tools/quadrenderer.h"

namespace TUMVis {
    const std::string DepthDarkening::loggerCat_ = "TUMVis.modules.vis.DepthDarkening";

    DepthDarkening::DepthDarkening(GenericProperty<tgt::ivec2>& canvasSize)
        : VisualizationProcessor(canvasSize)
        , _inputImage("InputImage", "Input Image", "", DataNameProperty::READ)
        , _outputImage("OutputImage", "Output Image", "dd.output", DataNameProperty::WRITE)
        , _sigma("Sigma", "Sigma of Gaussian Filter", 2.f, 0.f, 10.f)
        , _lambda("Lambda", "Strength of Depth Darkening Effect", 10.f, 0.f, 50.f)
        , _useColorCoding("UseColorCoding", "Cold/Warm Color Coding", false, InvalidationLevel::INVALID_SHADER)
        , _coldColor("ColdColor", "Cold Color (Far Objects)", tgt::vec3(0.f, 0.f, 1.f), tgt::vec3(0.f), tgt::vec3(1.f))
        , _warmColor("WarmColor", "Warm Color (Near Objects)", tgt::vec3(1.f, 0.f, 0.f), tgt::vec3(0.f), tgt::vec3(1.f))
        , _shader(0)
    {
        addProperty(&_inputImage);
        addProperty(&_outputImage);
        addProperty(&_sigma);
        addProperty(&_lambda);
        addProperty(&_useColorCoding);
        addProperty(&_coldColor);
        addProperty(&_warmColor);
    }

    DepthDarkening::~DepthDarkening() {

    }

    void DepthDarkening::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/vis/depthdarkening.frag", generateHeader(), false);
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void DepthDarkening::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void DepthDarkening::process(DataContainer& data) {
        DataContainer::ScopedTypedData<ImageDataRenderTarget> inputImage(data, _inputImage.getValue());

        if (inputImage != 0) {
            if (_invalidationLevel.isInvalidShader()) {
                _shader->setHeaders(generateHeader());
                _shader->rebuild();
            }

            // TODO: const cast is ugly...
            const_cast<tgt::Texture*>(inputImage->getDepthTexture())->downloadTexture();
            float* pixels = (float*)inputImage->getDepthTexture()->getPixelData();
            float curDepth = *(pixels);
            float minDepth = curDepth;
            float maxDepth = curDepth;
            size_t numPixels = inputImage->getNumElements();
            for (size_t i = 1; i < numPixels; ++i) {
                curDepth = pixels[i];
                minDepth = std::min(minDepth, curDepth);
                maxDepth = std::max(maxDepth, curDepth);
            }

            ImageDataRenderTarget* tempTarget = new ImageDataRenderTarget(tgt::svec3(_renderTargetSize.getValue(), 1));
            ImageDataRenderTarget* outputTarget = new ImageDataRenderTarget(tgt::svec3(_renderTargetSize.getValue(), 1));
            glPushAttrib(GL_ALL_ATTRIB_BITS);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_ALWAYS);

            _shader->activate();
            tgt::TextureUnit colorUnit, depthUnit, pass2DepthUnit;
            inputImage->bind(_shader, &colorUnit, &depthUnit);
            inputImage->bind(_shader, 0, &pass2DepthUnit, "", "_depthPass2Texture");
            
            _shader->setUniform("_viewportSizeRCP", 1.f / tgt::vec2(_renderTargetSize.getValue()));
            _shader->setUniform("_direction", tgt::vec2(1.f, 0.f));
            _shader->setUniform("_sigma", _sigma.getValue());
            _shader->setUniform("_lambda", _lambda.getValue());
            _shader->setUniform("_minDepth", minDepth);
            _shader->setUniform("_maxDepth", maxDepth);
            if (_useColorCoding.getValue()) {
                _shader->setUniform("_coldColor", _coldColor.getValue());
                _shader->setUniform("_warmColor", _warmColor.getValue());
            }

            tempTarget->activate();
            LGL_ERROR;
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            QuadRdr.renderQuad();
            tempTarget->deactivate();

            inputImage->bind(_shader, &colorUnit, &depthUnit);
            tempTarget->bind(_shader, 0, &pass2DepthUnit, "", "_depthPass2Texture");
            _shader->setUniform("_direction", tgt::vec2(0.f, 1.f));
            
            outputTarget->activate();
            LGL_ERROR;
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            QuadRdr.renderQuad();
            outputTarget->deactivate();

            _shader->deactivate();
            tgt::TextureUnit::setZeroUnit();
            glPopAttrib();
            LGL_ERROR;

            data.addData(_outputImage.getValue() + "temp", tempTarget);
            data.addData(_outputImage.getValue(), outputTarget);
            _outputImage.issueWrite();
        }
        else {
            LERROR("No suitable input image found.");
        }

        _invalidationLevel.setValid();
    }

    std::string DepthDarkening::generateHeader() const {
        if (_useColorCoding.getValue())
            return "#define USE_COLORCODING\n";
        else
            return "";
    }

}

