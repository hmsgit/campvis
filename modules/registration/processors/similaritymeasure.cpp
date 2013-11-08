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
    static const GenericOption<nlopt::algorithm> optimizers[3] = {
        GenericOption<nlopt::algorithm>("cobyla", "COBYLA", nlopt::LN_COBYLA),
        GenericOption<nlopt::algorithm>("newuoa", "NEWUOA", nlopt::LN_NEWUOA),
        GenericOption<nlopt::algorithm>("neldermead", "Nelder-Mead Simplex", nlopt::LN_NELDERMEAD)
    };

    const std::string SimilarityMeasure::loggerCat_ = "CAMPVis.modules.vis.SimilarityMeasure";

    SimilarityMeasure::SimilarityMeasure()
        : VisualizationProcessor(0)
        , p_referenceId("ReferenceId", "Reference Image", "", DataNameProperty::READ, AbstractProcessor::VALID)
        , p_movingId("MovingId", "Moving Image", "", DataNameProperty::READ, AbstractProcessor::VALID)
        , p_translation("Translation", "Moving Image Translation", tgt::vec3(0.f), tgt::vec3(-1000.f), tgt::vec3(1000.f), tgt::vec3(1.f), tgt::vec3(1.f))
        , p_rotation("Rotation", "Moving Image Rotation", tgt::vec3(0.f), tgt::vec3(-tgt::PIf), tgt::vec3(tgt::PIf), tgt::vec3(.1f), tgt::vec3(2.f))
        , p_viewportSize("ViewportSize", "Viewport Size", tgt::ivec2(1), tgt::ivec2(1), tgt::ivec2(1000), tgt::ivec2(1), AbstractProcessor::VALID)
        , p_computeSimilarity("ComputeSimilarity", "Compute Similarity")
        , p_optimizer("Optimizer", "Optimizer", optimizers, 3)
        , p_performOptimization("PerformOptimization", "Perform Optimization", AbstractProcessor::INVALID_RESULT | PERFORM_OPTIMIZATION)
        , _shader(0)
        , _glr(0)
    {
        addProperty(&p_referenceId);
        addProperty(&p_movingId);
        addProperty(&p_translation);
        addProperty(&p_rotation);
        addProperty(&p_computeSimilarity);
        addProperty(&p_optimizer);
        addProperty(&p_performOptimization);

        _viewportSizeProperty = &p_viewportSize;
    }

    SimilarityMeasure::~SimilarityMeasure() {

    }

    void SimilarityMeasure::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/registration/glsl/similaritymeasure.frag", "", false);
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");

        _glr = new GlReduction();
    }

    void SimilarityMeasure::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);

        delete _glr;
        _glr = 0;
    }

    void SimilarityMeasure::process(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation referenceImage(data, p_referenceId.getValue());
        ImageRepresentationGL::ScopedRepresentation movingImage(data, p_movingId.getValue());

        if (referenceImage != 0 && movingImage != 0) {
            if (getInvalidationLevel() & PERFORM_OPTIMIZATION) {
                performOptimization(referenceImage, movingImage);
            }

            float similarity = computeSimilarity(referenceImage, movingImage, p_translation.getValue(), p_rotation.getValue());
            LDEBUG("Similarity Measure: " << similarity);
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

    void SimilarityMeasure::performOptimization(const ImageRepresentationGL* referenceImage, const ImageRepresentationGL* movingImage) {
        tgtAssert(referenceImage != 0, "Reference Image must not be 0.");
        tgtAssert(movingImage != 0, "Moving Image must not be 0.");

        MyFuncData_t mfd = { this, referenceImage, movingImage, 0 };

        nlopt::opt opt(p_optimizer.getOptionValue(), 6);
        opt.set_min_objective(&SimilarityMeasure::optimizerFunc, &mfd);
        opt.set_xtol_rel(1e-4);

        std::vector<double> x(6);
        x[0] = p_translation.getValue().x;
        x[1] = p_translation.getValue().y;
        x[2] = p_translation.getValue().z;
        x[3] = p_rotation.getValue().x;
        x[4] = p_rotation.getValue().y;
        x[5] = p_rotation.getValue().z;

        std::vector<double> stepSize(6);
        stepSize[0] = 8.0;
        stepSize[1] = 8.0;
        stepSize[2] = 8.0;
        stepSize[3] = 0.5;
        stepSize[4] = 0.5;
        stepSize[5] = 0.5;
        opt.set_initial_step(stepSize);

        double minf;
        nlopt::result result = opt.optimize(x, minf);

        if (result >= nlopt::SUCCESS) {
            LDEBUG("Optimization successful, took " << mfd._count << " steps.");
            p_translation.setValue(tgt::vec3(x[0], x[1], x[2]));
            p_rotation.setValue(tgt::vec3(x[3], x[4], x[5]));
        }

        validate(PERFORM_OPTIMIZATION);
    }

    float SimilarityMeasure::computeSimilarity(const ImageRepresentationGL* referenceImage, const ImageRepresentationGL* movingImage, const tgt::vec3& translation, const tgt::vec3& rotation) {
        tgtAssert(referenceImage != 0, "Reference Image must not be 0.");
        tgtAssert(movingImage != 0, "Moving Image must not be 0.");

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

        tgt::mat4 registrationMatrix = tgt::mat4::createTranslation(translation) 
            * tgt::mat4::createRotationZ(rotation.z)
            * tgt::mat4::createRotationY(rotation.y)
            * tgt::mat4::createRotationX(rotation.x);

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
        //data.addData("All glory to the HYPNOTOAD!", new RenderData(_fbo));
        _fbo->detachAll();
        _fbo->deactivate();

        // reduce the juice
        float similarity = _glr->reduce(similarityTex);

        delete similarityTex;

        tgt::TextureUnit::setZeroUnit();
        LGL_ERROR;

        return similarity;
    }

    double SimilarityMeasure::optimizerFunc(const std::vector<double>& x, std::vector<double>& grad, void* my_func_data) {
        tgtAssert(x.size() == 6, "Must have 6 values in x.");
        tgtAssert(my_func_data != 0, "my_func_data must not be 0");

        MyFuncData_t* mfd = static_cast<MyFuncData_t*>(my_func_data);
        ++mfd->_count;
        tgt::vec3 translation(x[0], x[1], x[2]);
        tgt::vec3 rotation(x[3], x[4], x[5]);
        float similarity = mfd->_object->computeSimilarity(mfd->_reference, mfd->_moving, translation, rotation);

        LDEBUG(translation << rotation << " : " << similarity);

        return similarity;
    }


}
