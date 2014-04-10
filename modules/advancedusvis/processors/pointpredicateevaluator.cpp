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

#include "pointpredicateevaluator.h"
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
    const std::string PointPredicateEvaluator::loggerCat_ = "CAMPVis.modules.vis.PointPredicateEvaluator";


    PointPredicateEvaluator::PointPredicateEvaluator(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputImage("InputImage", "Input Image", "", DataNameProperty::READ)
        , p_inputLabels("InputLabels", "Input Label Image", "", DataNameProperty::READ)
        , p_inputSnr("InputSnr", "Input SNR", "", DataNameProperty::READ)
        , p_inputVesselness("InputVesselness", "Input Vesselness", "", DataNameProperty::READ)
        , p_inputConfidence("InputConfidence", "Input Confidence", "", DataNameProperty::READ)
        , p_outputImage("OutputImage", "Output Image", "predicatemask", DataNameProperty::WRITE)
        , p_camera("Camera", "Camera", tgt::Camera())
        , p_histogram("PredicateHistogram", "Point Predicate Histogram")
        , _shader(0)
    {
        addProperty(p_inputImage, INVALID_PROPERTIES | INVALID_RESULT);
        addProperty(p_inputLabels, INVALID_PROPERTIES | INVALID_RESULT);
        addProperty(p_inputSnr);
        addProperty(p_inputVesselness);
        addProperty(p_inputConfidence);
        addProperty(p_outputImage);

        addProperty(p_camera);
        addProperty(p_histogram);
    }

    PointPredicateEvaluator::~PointPredicateEvaluator() {

    }

    void PointPredicateEvaluator::init() {
        VisualizationProcessor::init();

        _shader = ShdrMgr.loadWithCustomGlslVersion("core/glsl/passthrough.vert", "", "modules/advancedusvis/glsl/pointpredicateevaluator.frag", generateHeader(), "400");
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
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
            const tgt::svec3& size = img->getSize();
            tgt::ivec2 viewportSize = size.xy();

            tgt::TextureUnit inputUnit, labelUnit, snrUnit, vesselnessUnit, confidenceUnit;
            inputUnit.activate();

            const tgt::Texture* tex = img->getTexture();
            if (tex->getFilter() != tgt::Texture::MIPMAP) {
                const_cast<tgt::Texture*>(tex)->setFilter(tgt::Texture::MIPMAP);
                LGL_ERROR;
                glGenerateMipmap(GL_TEXTURE_3D);
                LGL_ERROR;
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
                LGL_ERROR;
                glTexParameteri(GL_TEXTURE_3D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
                LGL_ERROR;
            }

            // create texture for result
            tgt::Texture* resultTexture = new tgt::Texture(0, tgt::ivec3(size), GL_RED_INTEGER, GL_R8UI, GL_UNSIGNED_BYTE, tgt::Texture::NEAREST);
            resultTexture->uploadTexture();
            resultTexture->setWrapping(tgt::Texture::CLAMP);

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
            
            tgt::TextureUnit::setZeroUnit();
            LGL_ERROR;
        }
        else {
            LERROR("No suitable input image found.");
        }

        validate(INVALID_RESULT);
    }

    void PointPredicateEvaluator::updateProperties(DataContainer dataContainer) {
        validate(AbstractProcessor::INVALID_PROPERTIES);
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
        validate(INVALID_SHADER);
    }

}
