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

#include "pointpredicateevaluator.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"

#include "core/datastructures/cameradata.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorbackground.h"

#include "core/classification/simpletransferfunction.h"
#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

#include "core/tools/quadrenderer.h"

namespace campvis {
    const std::string PointPredicateEvaluator::loggerCat_ = "CAMPVis.modules.vis.PointPredicateEvaluator";


    PointPredicateEvaluator::PointPredicateEvaluator(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputImage("InputImage", "Input Image", "", DataNameProperty::READ)
        , p_inputLabels("InputLabels", "Input Label Image", "", DataNameProperty::READ)
        , p_inputSnr("InputSnr", "Input SNR", "", DataNameProperty::READ)
        , p_inputVesselness("InputVesselness", "Input Vesselness", "", DataNameProperty::READ)
        , p_inputConfidence("InputConfidence", "Input Confidence", "", DataNameProperty::READ)
        , p_camera("Camera", "Camera ID", "camera", DataNameProperty::READ)
        , p_outputImage("OutputImage", "Output Image", "predicatemask", DataNameProperty::WRITE)
        , p_histogram("PredicateHistogram", "Point Predicate Histogram")
        , _shader(0)
    {
        addProperty(p_inputImage, INVALID_PROPERTIES | INVALID_RESULT);
        addProperty(p_inputLabels, INVALID_PROPERTIES | INVALID_RESULT);
        addProperty(p_inputSnr);
        addProperty(p_inputVesselness);
        addProperty(p_inputConfidence);
        addProperty(p_camera);
        addProperty(p_outputImage);

        addProperty(p_histogram);
    }

    PointPredicateEvaluator::~PointPredicateEvaluator() {

    }

    void PointPredicateEvaluator::init() {
        p_histogram.getPredicateHistogram()->setPredicateFunctionArgumentString("in float intensity, in float gradientMagnitude, in float gradientAngle, float viewAngle, in float snr, in float vesselness, in float confidence, in uint label");
        VisualizationProcessor::init();

        _shader = ShdrMgr.loadWithCustomGlslVersion("core/glsl/passthrough.vert", "", "modules/advancedusvis/glsl/pointpredicateevaluator.frag", generateHeader(), "400");
        p_histogram.s_headerChanged.connect(this, &PointPredicateEvaluator::onHistogramHeaderChanged);
    }

    void PointPredicateEvaluator::deinit() {
        p_histogram.s_headerChanged.disconnect(this);
        ShdrMgr.dispose(_shader);
        VisualizationProcessor::deinit();
    }

    void PointPredicateEvaluator::updateResult(DataContainer& dataContainer) {
        ImageRepresentationGL::ScopedRepresentation img(dataContainer, p_inputImage.getValue());
        ImageRepresentationGL::ScopedRepresentation labels(dataContainer, p_inputLabels.getValue());
        ImageRepresentationGL::ScopedRepresentation snr(dataContainer, p_inputSnr.getValue());
        ImageRepresentationGL::ScopedRepresentation vesselness(dataContainer, p_inputVesselness.getValue());
        ImageRepresentationGL::ScopedRepresentation confidence(dataContainer, p_inputConfidence.getValue());

        if (img && labels && snr && vesselness && confidence) {
            const cgt::svec3& size = img->getSize();
            cgt::ivec2 viewportSize = size.xy();

            cgt::TextureUnit inputUnit, labelUnit, snrUnit, vesselnessUnit, confidenceUnit;
            inputUnit.activate();

            const cgt::Texture* tex = img->getTexture();
            if (tex->getFilter() != cgt::Texture::MIPMAP) {
                const_cast<cgt::Texture*>(tex)->setFilter(cgt::Texture::MIPMAP);
                LGL_ERROR;
                glGenerateMipmap(GL_TEXTURE_3D);
                LGL_ERROR;
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                LGL_ERROR;
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                LGL_ERROR;
            }

            // create texture for result
            cgt::Texture* resultTexture = new cgt::Texture(GL_TEXTURE_3D, cgt::ivec3(size), GL_R8UI, cgt::Texture::NEAREST);
            resultTexture->setWrapping(cgt::Texture::CLAMP);

            // activate shader and bind textures
            _shader->activate();
            _shader->setIgnoreUniformLocationError(true);
            p_histogram.getPredicateHistogram()->setupRenderShader(_shader);
            _shader->setIgnoreUniformLocationError(false);

            img->bind(_shader, inputUnit);
            labels->bind(_shader, labelUnit, "_labels", "_labelsParams");
            snr->bind(_shader, snrUnit, "_snr", "_snrParams");
            vesselness->bind(_shader, vesselnessUnit, "_vesselness", "_vesselnessParams");
            confidence->bind(_shader, confidenceUnit, "_confidence", "_confidenceParams");

            // activate FBO and attach texture
            _fbo->activate();
            glViewport(0, 0, static_cast<GLsizei>(viewportSize.x), static_cast<GLsizei>(viewportSize.y));

            // render quad to compute difference measure by shader
            for (int z = 0; z < static_cast<int>(size.z); ++z) {
                float zTexCoord = static_cast<float>(z)/static_cast<float>(size.z) + .5f/static_cast<float>(size.z);
                _shader->setUniform("_zTexCoord", zTexCoord);
                _fbo->attachTexture(resultTexture, GL_COLOR_ATTACHMENT0, 0, z);
                QuadRdr.renderQuad();
            }

            _fbo->detachAll();
            _fbo->deactivate();
            _shader->deactivate();

            // put resulting image into DataContainer
            ImageData* id = new ImageData(3, size, 1);
            ImageRepresentationGL::create(id, resultTexture);
            id->setMappingInformation(img->getParent()->getMappingInformation());
            dataContainer.addData(p_outputImage.getValue(), id);
            
            cgt::TextureUnit::setZeroUnit();
            LGL_ERROR;
        }
        else {
            LDEBUG("No suitable input image found.");
        }
    }

    void PointPredicateEvaluator::updateProperties(DataContainer dataContainer) {
    }

    std::string PointPredicateEvaluator::generateHeader() const {
        std::string toReturn = p_histogram.getPredicateHistogram()->getGlslHeader();
        return toReturn;
    }

    void PointPredicateEvaluator::onHistogramHeaderChanged() {
        invalidate(INVALID_SHADER);
    }

    void PointPredicateEvaluator::updateShader() {
        _shader->setHeaders(generateHeader());
        _shader->rebuild();
    }

}
