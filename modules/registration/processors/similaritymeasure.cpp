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

#include "similaritymeasure.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/facegeometry.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorbackground.h"

#include "core/classification/simpletransferfunction.h"

#include "core/tools/glreduction.h"
#include "core/tools/quadrenderer.h"

namespace campvis {

    const std::string SimilarityMeasure::loggerCat_ = "CAMPVis.modules.vis.SimilarityMeasure";

    SimilarityMeasure::SimilarityMeasure()
        : VisualizationProcessor(0)
        , p_referenceId("ReferenceId", "Reference Image", "", DataNameProperty::READ, AbstractProcessor::VALID)
        , p_movingId("MovingId", "Moving Image", "", DataNameProperty::READ, AbstractProcessor::VALID)
        , p_translation("Translation", "Moving Image Translation", tgt::vec3(0.f), tgt::vec3(-1000.f), tgt::vec3(1000.f), tgt::vec3(1.f), tgt::vec3(1.f))
        , p_rotation("Rotation", "Moving Image Rotation", tgt::vec3(0.f), tgt::vec3(-tgt::PIf), tgt::vec3(tgt::PIf), tgt::vec3(.1f), tgt::vec3(2.f))
        , p_viewportSize("ViewportSize", "Viewport Size", tgt::ivec2(1), tgt::ivec2(1), tgt::ivec2(1000), tgt::ivec2(1), AbstractProcessor::VALID)
        , p_compute("ComputeSimilarity", "Compute Similarity")
        , _shader(0)
    {
        addProperty(&p_referenceId);
        addProperty(&p_movingId);
        addProperty(&p_translation);
        addProperty(&p_rotation);
        addProperty(&p_compute);

        _viewportSizeProperty = &p_viewportSize;
    }

    SimilarityMeasure::~SimilarityMeasure() {

    }

    void SimilarityMeasure::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/registration/glsl/similaritymeasure.frag", "", false);
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void SimilarityMeasure::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void SimilarityMeasure::process(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation referenceImage(data, p_referenceId.getValue());
        ImageRepresentationGL::ScopedRepresentation movingImage(data, p_movingId.getValue());

        if (referenceImage != 0 && movingImage != 0) {
            const tgt::svec3& size = referenceImage->getSize();
            p_viewportSize.setValue(size.xy());
            
            // reserve texture units
            tgt::TextureUnit referenceUnit, movingUnit;
            referenceUnit.activate();

            // create temporary texture for result
            tgt::Texture* similarityTex = new tgt::Texture(0, tgt::ivec3(p_viewportSize.getValue(), 1), GL_RGBA, GL_RGBA32F, GL_FLOAT, tgt::Texture::NEAREST);
            similarityTex->uploadTexture();
            similarityTex->setWrapping(tgt::Texture::CLAMP);

            // activate FBO and attach texture
            _fbo->activate();
            const tgt::ivec2& windowSize = p_viewportSize.getValue();
            glViewport(0, 0, static_cast<GLsizei>(windowSize.x), static_cast<GLsizei>(windowSize.y));
            _fbo->attachTexture(similarityTex);
            //_fbo->attachTexture(&similarityTex);
            LGL_ERROR;

            // bind input images
            _shader->activate();
            referenceImage->bind(_shader, referenceUnit, "_referenceTexture", "_referenceTextureParams");
            movingImage->bind(_shader, movingUnit, "_movingTexture", "_movingTextureParams");

            tgt::mat4 registrationMatrix = tgt::mat4::createTranslation(p_translation.getValue()) 
                                         * tgt::mat4::createRotationZ(p_rotation.getValue().z)
                                         * tgt::mat4::createRotationY(p_rotation.getValue().y)
                                         * tgt::mat4::createRotationX(p_rotation.getValue().x);

            const tgt::mat4& w2t = movingImage->getParent()->getMappingInformation().getWorldToTextureMatrix();
            const tgt::mat4& t2w = referenceImage->getParent()->getMappingInformation().getTextureToWorldMatrix();
            registrationMatrix = w2t * registrationMatrix * t2w;

            tgt::mat4 registrationInverse;
            if (! registrationMatrix.invert(registrationInverse))
                tgtAssert(false, "Could not invert registration matrix. This should not happen!");

            // render quad to compute similarity measure by shader
            //_shader->setUniform("_registrationMatrix", registrationMatrix);
            _shader->setUniform("_registrationInverse", registrationInverse);
            QuadRdr.renderQuad();
            _shader->deactivate();

            // detach texture and reduce it
            data.addData("All glory to the HYPNOTOAD!", new RenderData(_fbo));
            _fbo->detachAll();
            _fbo->deactivate();

            // reduce the juice
            GlReduction reducer;
            float similarity = reducer.reduce(similarityTex);

            LDEBUG("Similarity Measure: " << similarity);

            tgt::TextureUnit::setZeroUnit();
            LGL_ERROR;


        }
        else {
            LERROR("No suitable input image found.");
        }

        validate(INVALID_RESULT);
    }

    void SimilarityMeasure::updateProperties(DataContainer& dc) {
        ScopedTypedData<ImageData> referenceImage(dc, p_referenceId.getValue());
        if (referenceImage != 0) {
            p_viewportSize.setValue(referenceImage->getSize().xy());
        }

        validate(AbstractProcessor::INVALID_PROPERTIES);
    }
    
}
