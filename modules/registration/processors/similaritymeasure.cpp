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
#include "tgt/openglgarbagecollector.h"

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

    static const GenericOption<std::string> metrics[5] = {
        GenericOption<std::string>("SUM", "Sum"),
        GenericOption<std::string>("SAD", "SAD"),
        GenericOption<std::string>("SSD", "SSD"),
        GenericOption<std::string>("NCC", "NCC"),
        GenericOption<std::string>("SNR", "SNR")
    };

    const std::string SimilarityMeasure::loggerCat_ = "CAMPVis.modules.vis.SimilarityMeasure";

    SimilarityMeasure::SimilarityMeasure()
        : VisualizationProcessor(0)
        , p_referenceId("ReferenceId", "Reference Image", "", DataNameProperty::READ, AbstractProcessor::INVALID_PROPERTIES)
        , p_movingId("MovingId", "Moving Image", "", DataNameProperty::READ, AbstractProcessor::VALID)
        , p_clipX("clipX", "X Axis Clip Coordinates", tgt::ivec2(0), tgt::ivec2(0), tgt::ivec2(0))
        , p_clipY("clipY", "Y Axis Clip Coordinates", tgt::ivec2(0), tgt::ivec2(0), tgt::ivec2(0))
        , p_clipZ("clipZ", "Z Axis Clip Coordinates", tgt::ivec2(0), tgt::ivec2(0), tgt::ivec2(0))
        , p_applyMask("ApplyMask", "Apply Mask", true)
        , p_translation("Translation", "Moving Image Translation", tgt::vec3(0.f), tgt::vec3(-100.f), tgt::vec3(100.f), tgt::vec3(1.f), tgt::vec3(5.f))
        , p_rotation("Rotation", "Moving Image Rotation", tgt::vec3(0.f), tgt::vec3(-tgt::PIf), tgt::vec3(tgt::PIf), tgt::vec3(.01f), tgt::vec3(7.f))
        , p_viewportSize("ViewportSize", "Viewport Size", tgt::ivec2(1), tgt::ivec2(1), tgt::ivec2(1000), tgt::ivec2(1), AbstractProcessor::VALID)
        , p_metric("Metric", "Similarity Metric", metrics, 5)
        , p_computeSimilarity("ComputeSimilarity", "Compute Similarity")
        , p_optimizer("Optimizer", "Optimizer", optimizers, 3)
        , p_performOptimization("PerformOptimization", "Perform Optimization", AbstractProcessor::INVALID_RESULT | PERFORM_OPTIMIZATION)
        , p_differenceImageId("DifferenceImageId", "Difference Image", "difference", DataNameProperty::WRITE, AbstractProcessor::VALID)
        , p_computeDifferenceImage("ComputeDifferenceImage", "Compute Difference Image", AbstractProcessor::INVALID_RESULT | COMPUTE_DIFFERENCE_IMAGE)
        , p_forceStop("Force Stop", "Force Stop", AbstractProcessor::VALID)
        , _sadssdCostFunctionShader(0)
        , _nccsnrCostFunctionShader(0)
        , _differenceShader(0)
        , _glr(0)
        , _opt(0)
    {
        addProperty(&p_referenceId);
        addProperty(&p_movingId);

        addProperty(&p_clipX);
        addProperty(&p_clipY);
        addProperty(&p_clipZ);
        addProperty(&p_applyMask);

        addProperty(&p_translation);
        addProperty(&p_rotation);
        addProperty(&p_metric);
        addProperty(&p_computeSimilarity);

        addProperty(&p_differenceImageId);
        addProperty(&p_computeDifferenceImage);

        addProperty(&p_optimizer);
        addProperty(&p_performOptimization);
        addProperty(&p_forceStop);

        p_forceStop.s_clicked.connect(this, &SimilarityMeasure::forceStop);

        _viewportSizeProperty = &p_viewportSize;
    }

    SimilarityMeasure::~SimilarityMeasure() {

    }

    void SimilarityMeasure::init() {
        VisualizationProcessor::init();
        _sadssdCostFunctionShader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/registration/glsl/similaritymeasuresadssd.frag", "", false);
        _sadssdCostFunctionShader->setAttributeLocation(0, "in_Position");
        _sadssdCostFunctionShader->setAttributeLocation(1, "in_TexCoord");

        _nccsnrCostFunctionShader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/registration/glsl/similaritymeasurenccsnr.frag", "", false);
        _nccsnrCostFunctionShader->setAttributeLocation(0, "in_Position");
        _nccsnrCostFunctionShader->setAttributeLocation(1, "in_TexCoord");

        _differenceShader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/registration/glsl/differenceimage.frag", "", false);
        _differenceShader->setAttributeLocation(0, "in_Position");
        _differenceShader->setAttributeLocation(1, "in_TexCoord");

        _glr = new GlReduction(GlReduction::PLUS);
    }

    void SimilarityMeasure::deinit() {
        ShdrMgr.dispose(_sadssdCostFunctionShader);
        ShdrMgr.dispose(_nccsnrCostFunctionShader);
        ShdrMgr.dispose(_differenceShader);

        delete _glr;
        _glr = 0;

        delete _opt;
        _opt = 0;

        VisualizationProcessor::deinit();
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

            if (getInvalidationLevel() & COMPUTE_DIFFERENCE_IMAGE) 
                generateDifferenceImage(&data, referenceImage, movingImage, p_translation.getValue(), p_rotation.getValue());
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

            p_clipX.setMaxValue(tgt::ivec2(static_cast<int>(referenceImage->getSize().x), static_cast<int>(referenceImage->getSize().x)));
            p_clipY.setMaxValue(tgt::ivec2(static_cast<int>(referenceImage->getSize().y), static_cast<int>(referenceImage->getSize().y)));
            p_clipZ.setMaxValue(tgt::ivec2(static_cast<int>(referenceImage->getSize().z), static_cast<int>(referenceImage->getSize().z)));

            p_clipX.setValue(tgt::ivec2(0, static_cast<int>(referenceImage->getSize().x)));
            p_clipY.setValue(tgt::ivec2(0, static_cast<int>(referenceImage->getSize().y)));
            p_clipZ.setValue(tgt::ivec2(0, static_cast<int>(referenceImage->getSize().z)));
        }

        validate(AbstractProcessor::INVALID_PROPERTIES);
    }

    void SimilarityMeasure::performOptimization(const ImageRepresentationGL* referenceImage, const ImageRepresentationGL* movingImage) {
        tgtAssert(referenceImage != 0, "Reference Image must not be 0.");
        tgtAssert(movingImage != 0, "Moving Image must not be 0.");

        if (_opt != 0) {
            LWARNING("Optimization is already running...");
            return;
        }

        MyFuncData_t mfd = { this, referenceImage, movingImage, 0 };

        _opt = new nlopt::opt(p_optimizer.getOptionValue(), 6);
        if (p_metric.getOptionValue() == "NCC" || p_metric.getOptionValue() == "SNR") {
            _opt->set_max_objective(&SimilarityMeasure::optimizerFunc, &mfd);
        }
        else {
            _opt->set_min_objective(&SimilarityMeasure::optimizerFunc, &mfd);
        }
        _opt->set_xtol_rel(1e-4);

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
        _opt->set_initial_step(stepSize);

        double minf;
        nlopt::result result = nlopt::SUCCESS;
        try {
            result = _opt->optimize(x, minf);
        }
        catch (std::exception& e) {
            LERROR("Excpetion during optimization: " << e.what());
        }
        
        if (result >= nlopt::SUCCESS || result <= nlopt::ROUNDOFF_LIMITED) {
            LDEBUG("Optimization successful, took " << mfd._count << " steps.");
            p_translation.setValue(tgt::vec3(x[0], x[1], x[2]));
            p_rotation.setValue(tgt::vec3(x[3], x[4], x[5]));
        }

        delete _opt;
        _opt = 0;

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

        // create temporary texture(s) for result
        tgt::Shader* leShader = _sadssdCostFunctionShader;
        tgt::Texture* similarityTex = 0;
        tgt::Texture* similarityTex2 = 0;
        similarityTex = new tgt::Texture(0, tgt::ivec3(p_viewportSize.getValue(), 1), GL_RGBA, GL_RGBA32F, GL_FLOAT, tgt::Texture::NEAREST);
        similarityTex->uploadTexture();
        similarityTex->setWrapping(tgt::Texture::CLAMP);
        if (p_metric.getOptionValue() == "NCC" || p_metric.getOptionValue() == "SNR") {
            similarityTex2 = new tgt::Texture(0, tgt::ivec3(p_viewportSize.getValue(), 1), GL_RGBA, GL_RGBA32F, GL_FLOAT, tgt::Texture::NEAREST);
            similarityTex2->uploadTexture();
            similarityTex2->setWrapping(tgt::Texture::CLAMP);
            leShader = _nccsnrCostFunctionShader;
        }

        // activate FBO and attach texture
        _fbo->activate();
        const tgt::ivec2& windowSize = p_viewportSize.getValue();
        glViewport(0, 0, static_cast<GLsizei>(windowSize.x), static_cast<GLsizei>(windowSize.y));
        _fbo->attachTexture(similarityTex);
        if (p_metric.getOptionValue() == "NCC" || p_metric.getOptionValue() == "SNR")
            _fbo->attachTexture(similarityTex2, GL_COLOR_ATTACHMENT1);
        LGL_ERROR;

        // bind input images
        leShader->activate();
        leShader->setUniform("_applyMask", p_applyMask.getValue());
        leShader->setUniform("_xClampRange", tgt::vec2(p_clipX.getValue()) / static_cast<float>(size.x));
        leShader->setUniform("_yClampRange", tgt::vec2(p_clipY.getValue()) / static_cast<float>(size.y));
        leShader->setUniform("_zClampRange", tgt::vec2(p_clipZ.getValue()) / static_cast<float>(size.z));
        referenceImage->bind(leShader, referenceUnit, "_referenceTexture", "_referenceTextureParams");
        movingImage->bind(leShader, movingUnit, "_movingTexture", "_movingTextureParams");

        tgt::mat4 registrationMatrix = tgt::mat4::createTranslation(translation) * euleranglesToMat4(rotation);
        tgt::mat4 registrationInverse;
        if (! registrationMatrix.invert(registrationInverse))
            tgtAssert(false, "Could not invert registration matrix. This should not happen!");

        const tgt::mat4& w2t = movingImage->getParent()->getMappingInformation().getWorldToTextureMatrix();
        const tgt::mat4& t2w = referenceImage->getParent()->getMappingInformation().getTextureToWorldMatrix();
        registrationInverse = w2t * registrationInverse * t2w;


        // render quad to compute similarity measure by shader
        leShader->setUniform("_registrationInverse", registrationInverse);
        if (p_metric.getOptionValue() == "NCC" || p_metric.getOptionValue() == "SNR") {
            static const GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1};
            glDrawBuffers(2, buffers);
            QuadRdr.renderQuad();
            glDrawBuffers(1, buffers);
        }
        else {
            QuadRdr.renderQuad();
        }
        
        leShader->deactivate();

        // detach texture and reduce it
        //data.addData("All glory to the HYPNOTOAD!", new RenderData(_fbo));
        _fbo->detachAll();
        _fbo->deactivate();

        // reduce the juice
        float toReturn = 0.f;
        if (p_metric.getOptionValue() == "NCC") {
            std::vector<float> similarities = _glr->reduce(similarityTex);
            std::vector<float> similarities2 = _glr->reduce(similarityTex2);

            if (similarities.size() >= 3 && similarities2.size() >= 3) {
                float countRCP = 1.f / similarities[0];
                float meanFixed = similarities[1] * countRCP;
                float meanMoving = similarities[2] * countRCP;
                float varFixed	= (similarities2[1] - (similarities[2] * similarities[2]) * countRCP) * countRCP;
                float _varMoving = (similarities2[0] - (similarities[1] * similarities[1]) * countRCP) * countRCP;

                float correlation = 0.0f;
                if (varFixed > 0.0f && _varMoving > 0.0f)
                {
                    correlation = (similarities2[2] - (similarities[1] * similarities[2]) * countRCP) * countRCP;
                    toReturn =  correlation / sqrt(varFixed * _varMoving);
                }
            }
        }
        else if (p_metric.getOptionValue() == "SNR") {
            std::vector<float> similarities = _glr->reduce(similarityTex);
            std::vector<float> similarities2 = _glr->reduce(similarityTex2);

            if (similarities.size() >= 4 && similarities2.size() >= 4) {
                float countRCP = 1.f / similarities[0];
                float signal = similarities[3] * countRCP;
                float noise = sqrt(similarities2[3] * countRCP);

                toReturn = signal/noise;
            }
        }
        else {
            std::vector<float> similarities = _glr->reduce(similarityTex);

            if (p_metric.getOptionValue() == "SUM")
                toReturn = similarities[0];
            else if (p_metric.getOptionValue() == "SAD")
                toReturn = similarities[1];
            else if (p_metric.getOptionValue() == "SSD")
                toReturn = similarities[2];
        }

        delete similarityTex;
        delete similarityTex2;
        tgt::TextureUnit::setZeroUnit();
        LGL_ERROR;

        return toReturn;
    }

    double SimilarityMeasure::optimizerFunc(const std::vector<double>& x, std::vector<double>& grad, void* my_func_data) {
        tgtAssert(x.size() == 6, "Must have 6 values in x.");
        tgtAssert(my_func_data != 0, "my_func_data must not be 0");

        MyFuncData_t* mfd = static_cast<MyFuncData_t*>(my_func_data);
        ++mfd->_count;
        tgt::vec3 translation(x[0], x[1], x[2]);
        tgt::vec3 rotation(x[3], x[4], x[5]);
        float similarity = mfd->_object->computeSimilarity(mfd->_reference, mfd->_moving, translation, rotation);
        GLGC.deleteGarbage();

        LDEBUG(translation << rotation << " : " << similarity);

        return similarity;
    }

    tgt::mat4 SimilarityMeasure::euleranglesToMat4(const tgt::vec3& eulerAngles) {
        float sinX = sin(eulerAngles.x);
        float cosX = cos(eulerAngles.x);
        float sinY = sin(eulerAngles.y);
        float cosY = cos(eulerAngles.y);
        float sinZ = sin(eulerAngles.z);
        float cosZ = cos(eulerAngles.z);

        tgt::mat4 toReturn(cosY * cosZ,   cosZ * sinX * sinY - cosX * sinZ,   sinX * sinZ + cosX * cosZ * sinY,   0.f,
                           cosY * sinZ,   sinX * sinY * sinZ + cosX * cosZ,   cosX * sinY * sinZ - cosZ * sinX,   0.f,
                           (-1) * sinY,   cosY * sinX,                        cosX * cosY,                        0.f,
                           0.f,           0.f,                                0.f,                                1.f);
        return toReturn;
    }

    void SimilarityMeasure::generateDifferenceImage(DataContainer* dc, const ImageRepresentationGL* referenceImage, const ImageRepresentationGL* movingImage, const tgt::vec3& translation, const tgt::vec3& rotation) {
        tgtAssert(dc != 0, "DataContainer must not be 0.");
        tgtAssert(referenceImage != 0, "Reference Image must not be 0.");
        tgtAssert(movingImage != 0, "Moving Image must not be 0.");

        const tgt::svec3& size = referenceImage->getSize();
        tgt::ivec2 viewportSize = size.xy();

        // reserve texture units
        tgt::TextureUnit referenceUnit, movingUnit;
        referenceUnit.activate();

        // create temporary texture for result
        tgt::Texture* similarityTex = new tgt::Texture(0, tgt::ivec3(size), GL_RED, GL_R32F, GL_FLOAT, tgt::Texture::NEAREST);
        similarityTex->uploadTexture();
        similarityTex->setWrapping(tgt::Texture::CLAMP);

        // activate FBO and attach texture
        _fbo->activate();
        glViewport(0, 0, static_cast<GLsizei>(viewportSize.x), static_cast<GLsizei>(viewportSize.y));
        LGL_ERROR;

        // bind input images
        _differenceShader->activate();
        _differenceShader->setUniform("_applyMask", p_applyMask.getValue());
        _differenceShader->setUniform("_xClampRange", tgt::vec2(p_clipX.getValue()) / static_cast<float>(size.x));
        _differenceShader->setUniform("_yClampRange", tgt::vec2(p_clipY.getValue()) / static_cast<float>(size.y));
        _differenceShader->setUniform("_zClampRange", tgt::vec2(p_clipZ.getValue()) / static_cast<float>(size.z));
        referenceImage->bind(_differenceShader, referenceUnit, "_referenceTexture", "_referenceTextureParams");
        movingImage->bind(_differenceShader, movingUnit, "_movingTexture", "_movingTextureParams");

        tgt::mat4 registrationMatrix = tgt::mat4::createTranslation(translation) * euleranglesToMat4(rotation);
        tgt::mat4 registrationInverse;
        if (! registrationMatrix.invert(registrationInverse))
            tgtAssert(false, "Could not invert registration matrix. This should not happen!");

        const tgt::mat4& w2t = movingImage->getParent()->getMappingInformation().getWorldToTextureMatrix();
        const tgt::mat4& t2w = referenceImage->getParent()->getMappingInformation().getTextureToWorldMatrix();
        registrationInverse = w2t * registrationInverse * t2w;


        // render quad to compute similarity measure by shader
        _differenceShader->setUniform("_registrationInverse", registrationInverse);

        for (int z = 0; z < size.z; ++z) {
            float texZ = static_cast<float>(z)/static_cast<float>(size.z) + .5f/static_cast<float>(size.z);
            _differenceShader->setUniform("_zTex", texZ);
            _fbo->attachTexture(similarityTex, GL_COLOR_ATTACHMENT0, 0, z);
            QuadRdr.renderQuad();
        }
        _differenceShader->deactivate();

        // detach texture and reduce it
        ImageData* id = new ImageData(3, size, 1);
        ImageRepresentationGL::create(id, similarityTex);
        id->setMappingInformation(referenceImage->getParent()->getMappingInformation());
        dc->addData(p_differenceImageId.getValue(), id);
        _fbo->deactivate();

        tgt::TextureUnit::setZeroUnit();
        LGL_ERROR;

        validate(COMPUTE_DIFFERENCE_IMAGE);
    }

    void SimilarityMeasure::forceStop() {
        if (_opt != 0)
            _opt->force_stop();

        validate(PERFORM_OPTIMIZATION);
    }


}
