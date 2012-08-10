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

#include "raycastingprocessor.h"

#include "tgt/logmanager.h"
#include "tgt/quadrenderer.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagedatagl.h"
#include "core/datastructures/imagedatarendertarget.h"
#include "core/datastructures/imagedataconverter.h"

#include "core/classification/simpletransferfunction.h"

namespace TUMVis {
    const std::string RaycastingProcessor::loggerCat_ = "TUMVis.modules.vis.RaycastingProcessor";

    RaycastingProcessor::RaycastingProcessor(GenericProperty<tgt::ivec2>& renderTargetSize, const std::string& fragmentShaderFileName, bool bindEntryExitDepthTextures)
        : VisualizationProcessor(renderTargetSize)
        , _sourceImageID("sourceImageID", "Input Image", "")
        , _entryImageID("entryImageID", "Output Entry Points Image", "")
        , _exitImageID("exitImageID", "Output Exit Points Image", "")
        , _transferFunction("transferFunction", "Transfer Function", new SimpleTransferFunction(256))
        , _samplingStepSize("samplingStepSize", "Sampling Step Size", .1f, 0.001f, 1.f)
        , _jitterEntryPoints("jitterEntryPoints", "Jitter Entry Points", true)
        , _fragmentShaderFilename(fragmentShaderFileName)
        , _shader(0)
        , _bindEntryExitDepthTextures(bindEntryExitDepthTextures)
    {
        addProperty(&_sourceImageID);
        addProperty(&_entryImageID);
        addProperty(&_exitImageID);
        addProperty(&_transferFunction);
        addProperty(&_samplingStepSize);
        addProperty(&_jitterEntryPoints);
    }

    RaycastingProcessor::~RaycastingProcessor() {

    }

    void RaycastingProcessor::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", _fragmentShaderFilename, "", false);
        _shader->setHeaders(generateHeader());
        _shader->rebuild();
    }

    void RaycastingProcessor::deinit() {
        ShdrMgr.dispose(_shader);
        _shader = 0;
        VisualizationProcessor::deinit();
    }

    void RaycastingProcessor::process(DataContainer& data) {
        DataContainer::ScopedTypedData<ImageDataGL> img(data, _sourceImageID.getValue());
        DataContainer::ScopedTypedData<ImageDataRenderTarget> entryPoints(data, _entryImageID.getValue());
        DataContainer::ScopedTypedData<ImageDataRenderTarget> exitPoints(data, _exitImageID.getValue());

        if (img != 0 && entryPoints != 0 && exitPoints != 0) {
            if (img->getDimensionality() == 3) {
                if (_invalidationLevel.isInvalidShader()) {
                    _shader->setHeaders(generateHeader());
                    _shader->rebuild();
                }

                glPushAttrib(GL_ALL_ATTRIB_BITS);
                _shader->activate();
                _shader->setUniform("_viewportSizeRCP", 1.f / tgt::vec2(_renderTargetSize.getValue()));
                _shader->setUniform("_jitterEntryPoints", _jitterEntryPoints.getValue());
                _shader->setUniform("_samplingStepSize", _samplingStepSize.getValue());

                tgt::TextureUnit volumeUnit, entryUnit, exitUnit, tfUnit;
                img->bind(_shader, volumeUnit, "_volume");
                _transferFunction.getTF()->bind(_shader, tfUnit);

                if (! _bindEntryExitDepthTextures) {
                    entryPoints->bind(_shader, &entryUnit, 0, "_entryPoints");
                    exitPoints->bind(_shader, &exitUnit, 0, "_exitPoints");
                    processImpl(data);
                }
                else {
                    tgt::TextureUnit entryUnitDepth, exitUnitDepth;
                    entryPoints->bind(_shader, &entryUnit, &entryUnitDepth, "_entryPoints", "_entryPointsDepth");
                    exitPoints->bind(_shader, &exitUnit, &exitUnitDepth, "_exitPoints", "_exitPointsDepth");
                    processImpl(data);
                }

                _shader->deactivate();
                tgt::TextureUnit::setZeroUnit();
                glPopAttrib();
                LGL_ERROR;
            }
            else {
                LERROR("Input image must have dimensionality of 3.");
            }
        }
        else {
            LERROR("No suitable input image found.");
        }

        _invalidationLevel.setValid();
    }

    std::string RaycastingProcessor::generateHeader() const {
        std::string toReturn;
        return toReturn;
    }

}
