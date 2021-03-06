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

#include "similaritymeasure.h"
#include "cgt/logmanager.h"
#include "cgt/shadermanager.h"
#include "cgt/textureunit.h"

#include "core/datastructures/facegeometry.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorbackground.h"

#include "core/classification/simpletransferfunction.h"

#include "core/tools/glreduction.h"
#include "core/tools/quadrenderer.h"

namespace campvis {
namespace registration {

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
        , p_referenceId("ReferenceId", "Reference Image", "", DataNameProperty::READ)
        , p_movingId("MovingId", "Moving Image", "", DataNameProperty::READ)
        , p_clipX("clipX", "X Axis Clip Coordinates", cgt::ivec2(0), cgt::ivec2(0), cgt::ivec2(0))
        , p_clipY("clipY", "Y Axis Clip Coordinates", cgt::ivec2(0), cgt::ivec2(0), cgt::ivec2(0))
        , p_clipZ("clipZ", "Z Axis Clip Coordinates", cgt::ivec2(0), cgt::ivec2(0), cgt::ivec2(0))
        , p_applyMask("ApplyMask", "Apply Mask", true)
        , p_translation("Translation", "Moving Image Translation", cgt::vec3(0.f), cgt::vec3(-100.f), cgt::vec3(100.f), cgt::vec3(1.f), cgt::vec3(5.f))
        , p_rotation("Rotation", "Moving Image Rotation", cgt::vec3(0.f), cgt::vec3(-cgt::PIf), cgt::vec3(cgt::PIf), cgt::vec3(.01f), cgt::vec3(7.f))
        , p_metric("Metric", "Similarity Metric", metrics, 5)
        , p_computeSimilarity("ComputeSimilarity", "Compute Similarity")
        , p_differenceImageId("DifferenceImageId", "Difference Image", "difference", DataNameProperty::WRITE)
        , p_computeDifferenceImage("ComputeDifferenceImage", "Compute Difference Image")
        , p_viewportSize("ViewportSize", "Viewport Size", cgt::ivec2(1), cgt::ivec2(1), cgt::ivec2(1000), cgt::ivec2(1))
        , _sadssdCostFunctionShader(0)
        , _nccsnrCostFunctionShader(0)
        , _differenceShader(0)
        , _glr(0)
    {
        addProperty(p_referenceId, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_movingId, VALID);

        addProperty(p_clipX);
        addProperty(p_clipY);
        addProperty(p_clipZ);
        addProperty(p_applyMask);

        addProperty(p_translation);
        addProperty(p_rotation);
        addProperty(p_metric);
        addProperty(p_computeSimilarity);

        addProperty(p_differenceImageId, VALID);
        addProperty(p_computeDifferenceImage, INVALID_RESULT | COMPUTE_DIFFERENCE_IMAGE);


        _viewportSizeProperty = &p_viewportSize;
    }

    SimilarityMeasure::~SimilarityMeasure() {

    }

    void SimilarityMeasure::init() {
        VisualizationProcessor::init();
        _sadssdCostFunctionShader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/registration/glsl/similaritymeasuresadssd.frag", "");
        _nccsnrCostFunctionShader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/registration/glsl/similaritymeasurenccsnr.frag", "");
        _differenceShader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/registration/glsl/differenceimage.frag", "");

        _glr = new GlReduction(GlReduction::PLUS);
    }

    void SimilarityMeasure::deinit() {
        ShdrMgr.dispose(_sadssdCostFunctionShader);
        ShdrMgr.dispose(_nccsnrCostFunctionShader);
        ShdrMgr.dispose(_differenceShader);

        delete _glr;
        _glr = 0;

        VisualizationProcessor::deinit();
    }

    void SimilarityMeasure::updateResult(DataContainer& data) {
        ImageRepresentationGL::ScopedRepresentation referenceImage(data, p_referenceId.getValue());
        ImageRepresentationGL::ScopedRepresentation movingImage(data, p_movingId.getValue());

        if (referenceImage != 0 && movingImage != 0) {
            float similarity = computeSimilarity(referenceImage, movingImage, p_translation.getValue(), p_rotation.getValue());
            LDEBUG("Similarity Measure: " << similarity);

            if (getInvalidationLevel() & COMPUTE_DIFFERENCE_IMAGE) 
                generateDifferenceImage(&data, referenceImage, movingImage, p_translation.getValue(), p_rotation.getValue());
        }
        else {
            LERROR("No suitable input image found.");
        }
    }

    void SimilarityMeasure::updateProperties(DataContainer& dc) {
        ScopedTypedData<ImageData> referenceImage(dc, p_referenceId.getValue());
        if (referenceImage != 0) {
            p_viewportSize.setValue(referenceImage->getSize().xy());

            p_clipX.setMaxValue(cgt::ivec2(static_cast<int>(referenceImage->getSize().x), static_cast<int>(referenceImage->getSize().x)));
            p_clipY.setMaxValue(cgt::ivec2(static_cast<int>(referenceImage->getSize().y), static_cast<int>(referenceImage->getSize().y)));
            p_clipZ.setMaxValue(cgt::ivec2(static_cast<int>(referenceImage->getSize().z), static_cast<int>(referenceImage->getSize().z)));

            p_clipX.setValue(cgt::ivec2(0, static_cast<int>(referenceImage->getSize().x)));
            p_clipY.setValue(cgt::ivec2(0, static_cast<int>(referenceImage->getSize().y)));
            p_clipZ.setValue(cgt::ivec2(0, static_cast<int>(referenceImage->getSize().z)));
        }
    }

    float SimilarityMeasure::computeSimilarity(const ImageRepresentationGL* referenceImage, const ImageRepresentationGL* movingImage, const cgt::vec3& translation, const cgt::vec3& rotation) {
        cgtAssert(referenceImage != 0, "Reference Image must not be 0.");
        cgtAssert(movingImage != 0, "Moving Image must not be 0.");

        const cgt::svec3& size = referenceImage->getSize();
        p_viewportSize.setValue(size.xy());

        // reserve texture units
        cgt::TextureUnit referenceUnit, movingUnit;
        referenceUnit.activate();

        // create temporary texture(s) for result
        cgt::Shader* leShader = _sadssdCostFunctionShader;
        cgt::Texture* similarityTex = 0;
        cgt::Texture* similarityTex2 = 0;
        similarityTex = new cgt::Texture(GL_TEXTURE_2D, cgt::ivec3(p_viewportSize.getValue(), 1), GL_RGBA32F, cgt::Texture::NEAREST);
        similarityTex->setWrapping(cgt::Texture::CLAMP_TO_EDGE);
        // NCC and SNR need a second texture and a different shader...
        if (p_metric.getOptionValue() == "NCC" || p_metric.getOptionValue() == "SNR") {
            similarityTex2 = new cgt::Texture(GL_TEXTURE_2D, cgt::ivec3(p_viewportSize.getValue(), 1), GL_RGBA32F, cgt::Texture::NEAREST);
            similarityTex2->setWrapping(cgt::Texture::CLAMP_TO_EDGE);
            leShader = _nccsnrCostFunctionShader;
        }

        // activate FBO and attach texture
        _fbo->activate();
        const cgt::ivec2& windowSize = p_viewportSize.getValue();
        glViewport(0, 0, static_cast<GLsizei>(windowSize.x), static_cast<GLsizei>(windowSize.y));
        _fbo->attachTexture(similarityTex);
        if (p_metric.getOptionValue() == "NCC" || p_metric.getOptionValue() == "SNR")
            _fbo->attachTexture(similarityTex2, GL_COLOR_ATTACHMENT1);
        LGL_ERROR;

        // bind input images
        leShader->activate();
        leShader->setUniform("_applyMask", p_applyMask.getValue());
        leShader->setUniform("_xClampRange", cgt::vec2(p_clipX.getValue()) / static_cast<float>(size.x));
        leShader->setUniform("_yClampRange", cgt::vec2(p_clipY.getValue()) / static_cast<float>(size.y));
        leShader->setUniform("_zClampRange", cgt::vec2(p_clipZ.getValue()) / static_cast<float>(size.z));
        referenceImage->bind(leShader, referenceUnit, "_referenceTexture", "_referenceTextureParams");
        movingImage->bind(leShader, movingUnit, "_movingTexture", "_movingTextureParams");

        // render quad to compute similarity measure by shader
        leShader->setUniform("_registrationInverse", computeRegistrationMatrix(referenceImage, movingImage, translation, rotation));
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
                float varFixed	= (similarities2[1] - (similarities[2] * similarities[2]) * countRCP) * countRCP;
                float _varMoving = (similarities2[0] - (similarities[1] * similarities[1]) * countRCP) * countRCP;

                if (varFixed > 0.0f && _varMoving > 0.0f) {
                    float correlation = (similarities2[2] - (similarities[1] * similarities[2]) * countRCP) * countRCP;
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
        cgt::TextureUnit::setZeroUnit();
        LGL_ERROR;

        return toReturn;
    }


    cgt::mat4 SimilarityMeasure::euleranglesToMat4(const cgt::vec3& eulerAngles) {
        float sinX = sin(eulerAngles.x);
        float cosX = cos(eulerAngles.x);
        float sinY = sin(eulerAngles.y);
        float cosY = cos(eulerAngles.y);
        float sinZ = sin(eulerAngles.z);
        float cosZ = cos(eulerAngles.z);

        cgt::mat4 toReturn(cosY * cosZ,   cosZ * sinX * sinY - cosX * sinZ,   sinX * sinZ + cosX * cosZ * sinY,   0.f,
                           cosY * sinZ,   sinX * sinY * sinZ + cosX * cosZ,   cosX * sinY * sinZ - cosZ * sinX,   0.f,
                           (-1) * sinY,   cosY * sinX,                        cosX * cosY,                        0.f,
                           0.f,           0.f,                                0.f,                                1.f);
        return toReturn;
    }

    void SimilarityMeasure::generateDifferenceImage(DataContainer* dc, const ImageRepresentationGL* referenceImage, const ImageRepresentationGL* movingImage, const cgt::vec3& translation, const cgt::vec3& rotation) {
        cgtAssert(dc != 0, "DataContainer must not be 0.");
        cgtAssert(referenceImage != 0, "Reference Image must not be 0.");
        cgtAssert(movingImage != 0, "Moving Image must not be 0.");

        const cgt::svec3& size = referenceImage->getSize();
        cgt::ivec2 viewportSize = size.xy();

        // reserve texture units
        cgt::TextureUnit referenceUnit, movingUnit;
        referenceUnit.activate();

        // create temporary texture for result
        cgt::Texture* differenceImage = new cgt::Texture(GL_TEXTURE_3D, cgt::ivec3(size), GL_R32F, cgt::Texture::LINEAR);
        differenceImage->setWrapping(cgt::Texture::CLAMP_TO_EDGE);

        // bind input images
        _differenceShader->activate();
        _differenceShader->setUniform("_applyMask", p_applyMask.getValue());
        _differenceShader->setUniform("_xClampRange", cgt::vec2(p_clipX.getValue()) / static_cast<float>(size.x));
        _differenceShader->setUniform("_yClampRange", cgt::vec2(p_clipY.getValue()) / static_cast<float>(size.y));
        _differenceShader->setUniform("_zClampRange", cgt::vec2(p_clipZ.getValue()) / static_cast<float>(size.z));
        referenceImage->bind(_differenceShader, referenceUnit, "_referenceTexture", "_referenceTextureParams");
        movingImage->bind(_differenceShader, movingUnit, "_movingTexture", "_movingTextureParams");
        _differenceShader->setUniform("_registrationInverse", computeRegistrationMatrix(referenceImage, movingImage, translation, rotation));

        // activate FBO and attach texture
        _fbo->activate();
        glViewport(0, 0, static_cast<GLsizei>(viewportSize.x), static_cast<GLsizei>(viewportSize.y));

        // render quad to compute difference measure by shader
        for (int z = 0; z < static_cast<int>(size.z); ++z) {
            float texZ = static_cast<float>(z)/static_cast<float>(size.z) + .5f/static_cast<float>(size.z);
            _differenceShader->setUniform("_zTex", texZ);
            _fbo->attachTexture(differenceImage, GL_COLOR_ATTACHMENT0, 0, z);
            QuadRdr.renderQuad();
        }
        _differenceShader->deactivate();
        _fbo->deactivate();

        // put difference image into DataContainer
        ImageData* id = new ImageData(3, size, 1);
        ImageRepresentationGL::create(id, differenceImage);
        id->setMappingInformation(referenceImage->getParent()->getMappingInformation());
        dc->addData(p_differenceImageId.getValue(), id);

        cgt::TextureUnit::setZeroUnit();
        LGL_ERROR;

        validate(COMPUTE_DIFFERENCE_IMAGE);
    }

    cgt::mat4 SimilarityMeasure::computeRegistrationMatrix(const ImageRepresentationGL* referenceImage, const ImageRepresentationGL* movingImage, const cgt::vec3& translation, const cgt::vec3& rotation) {
        cgt::mat4 registrationMatrix = cgt::mat4::createTranslation(translation) * euleranglesToMat4(rotation);
        cgt::mat4 registrationInverse;
        if (! registrationMatrix.invert(registrationInverse))
            cgtAssert(false, "Could not invert registration matrix. This should not happen!");

        cgt::Bounds movingBounds = movingImage->getParent()->getWorldBounds();
        cgt::vec3 halfDiagonal = movingBounds.getLLF() + (movingBounds.diagonal() / 2.f);
        const cgt::mat4& w2t = movingImage->getParent()->getMappingInformation().getWorldToTextureMatrix();
        const cgt::mat4& t2w = referenceImage->getParent()->getMappingInformation().getTextureToWorldMatrix();
        return w2t * cgt::mat4::createTranslation(halfDiagonal) * registrationInverse * cgt::mat4::createTranslation(-halfDiagonal) * t2w;
    }

}
}
