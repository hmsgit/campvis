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

#include "raycastingprocessor.h"

#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/renderdata.h"

#include "core/classification/simpletransferfunction.h"

namespace campvis {
    const std::string RaycastingProcessor::loggerCat_ = "CAMPVis.modules.vis.RaycastingProcessor";

    RaycastingProcessor::RaycastingProcessor(IVec2Property* viewportSizeProp, const std::string& fragmentShaderFileName, bool bindEntryExitDepthTextures)
        : VisualizationProcessor(viewportSizeProp)
        , p_sourceImageID("sourceImageID", "Input Image", "", DataNameProperty::READ)
        , p_entryImageID("entryImageID", "Input Entry Points Image", "", DataNameProperty::READ)
        , p_exitImageID("exitImageID", "Input Exit Points Image", "", DataNameProperty::READ)
        , p_camera("camera", "Camera")
        , p_transferFunction("TransferFunction", "Transfer Function", new SimpleTransferFunction(256))
        , p_samplingRate("SamplingRate", "Sampling Rate", 2.f, 0.1f, 10.f)
        , p_jitterEntryPoints("jitterEntryPoints", "Jitter Entry Points", true)
        , p_jitterStepSizeMultiplier("jitterStepSizeMultiplier", "Jitter Step Size Multiplier", .5f, .1f, 1.f)
        , _fragmentShaderFilename(fragmentShaderFileName)
        , _shader(0)
        , _bindEntryExitDepthTextures(bindEntryExitDepthTextures)
        , _sourceImageTimestamp(0)
    {
        addProperty(&p_sourceImageID);
        addProperty(&p_entryImageID);
        addProperty(&p_exitImageID);
        addProperty(&p_camera);  
        addProperty(&p_transferFunction);
        addProperty(&p_samplingRate);
        addProperty(&p_jitterEntryPoints);
        addProperty(&p_jitterStepSizeMultiplier);
    }

    RaycastingProcessor::~RaycastingProcessor() {

    }

    void RaycastingProcessor::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", _fragmentShaderFilename, generateHeader(), false);
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void RaycastingProcessor::deinit() {
        ShdrMgr.dispose(_shader);
        _shader = 0;
        VisualizationProcessor::deinit();
    }

    void RaycastingProcessor::process(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_sourceImageID.getValue());
        DataContainer::ScopedTypedData<RenderData> entryPoints(data, p_entryImageID.getValue());
        DataContainer::ScopedTypedData<RenderData> exitPoints(data, p_exitImageID.getValue());

        if (img != 0 && entryPoints != 0 && exitPoints != 0) {
            if (img->getDimensionality() == 3) {
                if (img.getDataHandle().getTimestamp() != _sourceImageTimestamp) {
                    // source DataHandle has changed
                    _sourceImageTimestamp = img.getDataHandle().getTimestamp();
                    p_transferFunction.getTF()->setImageHandle(img.getDataHandle());
                }

                if (hasInvalidShader()) {
                    _shader->setHeaders(generateHeader());
                    _shader->rebuild();
                    validate(INVALID_SHADER);
                }

                _shader->activate();

                _shader->setIgnoreUniformLocationError(true);
                decorateRenderProlog(data, _shader);
                _shader->setUniform("_viewportSizeRCP", 1.f / tgt::vec2(getEffectiveViewportSize()));
                _shader->setUniform("_jitterEntryPoints", p_jitterEntryPoints.getValue());
                _shader->setUniform("_jitterStepSizeMultiplier", p_jitterStepSizeMultiplier.getValue());

                float samplingStepSize = 1.f / (p_samplingRate.getValue() * tgt::max(img->getSize()));
                _shader->setUniform("_samplingStepSize", samplingStepSize);

                const tgt::Camera& cam = p_camera.getValue();
                float n = cam.getNearDist();
                float f = cam.getFarDist();
                _shader->setUniform("_cameraPosition", cam.getPosition());
                _shader->setUniform("const_to_z_e_1", 0.5f + 0.5f*((f+n)/(f-n)));
                _shader->setUniform("const_to_z_e_2", ((f-n)/(f*n)));
                _shader->setUniform("const_to_z_w_1", ((f*n)/(f-n)));
                _shader->setUniform("const_to_z_w_2", 0.5f*((f+n)/(f-n))+0.5f);
                _shader->setIgnoreUniformLocationError(false);

                tgt::TextureUnit volumeUnit, entryUnit, exitUnit, tfUnit;
                img->bind(_shader, volumeUnit, "_volume", "_volumeTextureParams");
                p_transferFunction.getTF()->bind(_shader, tfUnit);

                if (! _bindEntryExitDepthTextures) {
                    entryPoints->bindColorTexture(_shader, entryUnit, "_entryPoints", "_entryParams");
                    exitPoints->bindColorTexture(_shader, exitUnit, "_exitPoints", "_exitParams");
                    processImpl(data, img);
                }
                else {
                    tgt::TextureUnit entryUnitDepth, exitUnitDepth;
                    entryPoints->bind(_shader, entryUnit, entryUnitDepth, "_entryPoints", "_entryPointsDepth", "_entryParams");
                    exitPoints->bind(_shader, exitUnit, exitUnitDepth, "_exitPoints", "_exitPointsDepth", "_exitParams");
                    processImpl(data, img);
                }

                decorateRenderEpilog(_shader);
                _shader->deactivate();
                tgt::TextureUnit::setZeroUnit();
                LGL_ERROR;
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

    std::string RaycastingProcessor::generateHeader() const {
        std::string toReturn = getDecoratedHeader();
        return toReturn;
    }

}
