// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

#include "raycastingprocessor.h"

#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/renderdata.h"
#include "core/tools/glreduction.h"

#include "core/classification/simpletransferfunction.h"

namespace campvis {
    const std::string RaycastingProcessor::loggerCat_ = "CAMPVis.modules.vis.RaycastingProcessor";

    RaycastingProcessor::RaycastingProcessor(IVec2Property* viewportSizeProp, const std::string& fragmentShaderFileName, bool bindEntryExitDepthTextures, const std::string& customGlslVersion /*= ""*/)
        : VisualizationProcessor(viewportSizeProp)
        , p_sourceImageID("sourceImageID", "Input Image", "", DataNameProperty::READ)
        , p_entryImageID("entryImageID", "Input Entry Points Image", "", DataNameProperty::READ)
        , p_exitImageID("exitImageID", "Input Exit Points Image", "", DataNameProperty::READ)
        , p_targetImageID("targetImageID", "Output Image", "", DataNameProperty::WRITE)
        , p_camera("camera", "Camera")
        , p_transferFunction("TransferFunction", "Transfer Function", new SimpleTransferFunction(256))
        , p_jitterStepSizeMultiplier("jitterStepSizeMultiplier", "Jitter Step Size Multiplier", 1.f, 0.f, 1.f)
        , p_samplingRate("SamplingRate", "Sampling Rate", 2.f, 0.1f, 10.f, 0.1f)
        , _fragmentShaderFilename(fragmentShaderFileName)
        , _customGlslVersion(customGlslVersion)
        , _shader(0)
        , _bindEntryExitDepthTextures(bindEntryExitDepthTextures)
    {
        addProperty(p_sourceImageID, AbstractProcessor::INVALID_RESULT | AbstractProcessor::INVALID_PROPERTIES);
        addProperty(p_entryImageID);
        addProperty(p_exitImageID);
        addProperty(p_targetImageID);
        addProperty(p_camera);  
        addProperty(p_transferFunction);
        addProperty(p_jitterStepSizeMultiplier);
        addProperty(p_samplingRate);
    }

    RaycastingProcessor::~RaycastingProcessor() {

    }

    void RaycastingProcessor::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadWithCustomGlslVersion("core/glsl/passthrough.vert", "", _fragmentShaderFilename, generateHeader(), _customGlslVersion);
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");

        _minReduction = new GlReduction(GlReduction::MIN);
        _maxReduction = new GlReduction(GlReduction::MAX);
    }

    void RaycastingProcessor::deinit() {
        ShdrMgr.dispose(_shader);
        _shader = 0;

        delete _minReduction;
        delete _maxReduction;

        VisualizationProcessor::deinit();
    }

    void RaycastingProcessor::updateResult(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation img(data, p_sourceImageID.getValue());
        ScopedTypedData<RenderData> entryPoints(data, p_entryImageID.getValue());
        ScopedTypedData<RenderData> exitPoints(data, p_exitImageID.getValue());

        if (img != 0 && entryPoints != 0 && exitPoints != 0) {
            if (img->getDimensionality() == 3) {
                // little hack to support LOD texture lookup for the gradients:
                // if texture does not yet have mipmaps, create them.
                const tgt::Texture* tex = img->getTexture();
                if (tex->getFilter() != tgt::Texture::MIPMAP) {
                    const_cast<tgt::Texture*>(tex)->setFilter(tgt::Texture::MIPMAP);
                    glGenerateMipmap(GL_TEXTURE_3D);
                    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                    glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                    LGL_ERROR;
                }

                _shader->activate();
                _shader->setIgnoreUniformLocationError(true);

                // Compute min/max depth if needed by shader
                if (_shader->getUniformLocation("_minDepth") != -1) {
                    _shader->deactivate();
                    float minDepth = _minReduction->reduce(entryPoints->getDepthTexture()).front();
                    _shader->activate();

                    _shader->setUniform("_minDepth", minDepth);
                }
                if (_shader->getUniformLocation("_maxDepth") != -1) {
                    _shader->deactivate();
                    float maxDepth = _maxReduction->reduce(exitPoints->getDepthTexture()).front();
                    _shader->activate();

                    _shader->setUniform("_maxDepth", maxDepth);
                }

                decorateRenderProlog(data, _shader);
                _shader->setUniform("_viewportSizeRCP", 1.f / tgt::vec2(getEffectiveViewportSize()));
                _shader->setUniform("_jitterStepSizeMultiplier", p_jitterStepSizeMultiplier.getValue());

                // compute sampling step size relative to volume size
                float samplingStepSize = 1.f / (p_samplingRate.getValue() * tgt::max(img->getSize()));
                _shader->setUniform("_samplingStepSize", samplingStepSize);

                // compute and set camera parameters
                const tgt::Camera& cam = p_camera.getValue();
                float n = cam.getNearDist();
                float f = cam.getFarDist();
                _shader->setUniform("_cameraPosition", cam.getPosition());
                _shader->setUniform("const_to_z_e_1", 0.5f + 0.5f*((f+n)/(f-n)));
                _shader->setUniform("const_to_z_e_2", ((f-n)/(f*n)));
                _shader->setUniform("const_to_z_w_1", ((f*n)/(f-n)));
                _shader->setUniform("const_to_z_w_2", 0.5f*((f+n)/(f-n))+0.5f);
                _shader->setIgnoreUniformLocationError(false);

                // bind input textures
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
            LDEBUG("No suitable input image found.");
        }
    }

    std::string RaycastingProcessor::generateHeader() const {
        std::string toReturn = getDecoratedHeader();
        return toReturn;
    }

    void RaycastingProcessor::updateProperties(DataContainer& dc) {
        ScopedTypedData<ImageData> img(dc, p_sourceImageID.getValue());
        p_transferFunction.setImageHandle(img.getDataHandle());
    }

    void RaycastingProcessor::updateShader() {
        _shader->setHeaders(generateHeader());
        _shader->rebuild();
    }

}
