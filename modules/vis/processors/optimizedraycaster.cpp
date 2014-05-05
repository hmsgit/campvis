// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "optimizedraycaster.h"

#include "core/tools/quadrenderer.h"
#include "core/datastructures/lightsourcedata.h"
#include "core/datastructures/imagerepresentationlocal.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorgradient.h"

#include <tbb/tbb.h>

namespace campvis {
    const std::string OptimizedRaycaster::loggerCat_ = "CAMPVis.modules.vis.OptimizedRaycaster";

    OptimizedRaycaster::OptimizedRaycaster(IVec2Property* viewportSizeProp)
        : RaycastingProcessor(viewportSizeProp, "modules/vis/glsl/optimizedraycaster.frag", true)
        , p_targetImageID("targetImageID", "Output Image", "", DataNameProperty::WRITE)
        , p_enableShading("EnableShading", "Enable Shading", true)
        , p_lightId("LightId", "Input Light Source", "lightsource", DataNameProperty::READ)
        , p_enableShadowing("EnableShadowing", "Enable Hard Shadows (Expensive!)", false)
        , p_shadowIntensity("ShadowIntensity", "Shadow Intensity", .5f, .0f, 1.f)
        , p_enableIntersectionRefinement("EnableIntersectionRefinement", "Enable Intersection Refinement", false)
        , p_useEmptySpaceSkipping("EnableEmptySpaceSkipping", "Enable Empty Space Skipping", true)
        , _bbv(0)
        , _t(0)
    {
        addDecorator(new ProcessorDecoratorGradient());

        addProperty(p_enableShading, INVALID_RESULT | INVALID_PROPERTIES | INVALID_SHADER);
        addProperty(p_lightId);
        addProperty(p_targetImageID);
        addProperty(p_enableIntersectionRefinement, INVALID_RESULT | INVALID_SHADER);
        addProperty(p_useEmptySpaceSkipping, INVALID_RESULT | INVALID_BBV);

        addProperty(p_enableShadowing, INVALID_RESULT | INVALID_SHADER | INVALID_PROPERTIES);
        addProperty(p_shadowIntensity);
        p_shadowIntensity.setVisible(false);

//         p_transferFunction.setInvalidationLevel(p_transferFunction.getInvalidationLevel() | INVALID_BBV);
//         p_sourceImageID.setInvalidationLevel(p_sourceImageID.getInvalidationLevel() | INVALID_BBV);

        decoratePropertyCollection(this);
    }

    OptimizedRaycaster::~OptimizedRaycaster() {
    }

    void OptimizedRaycaster::init() {
        RaycastingProcessor::init();

        invalidate(INVALID_BBV);
    }

    void OptimizedRaycaster::deinit() {
        delete _bbv;
        delete _t;
        RaycastingProcessor::deinit();
    }

    void OptimizedRaycaster::processImpl(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image) {
        tgt::TextureUnit bbvUnit;

        if (getInvalidationLevel() & INVALID_BBV) {
            DataHandle dh = DataHandle(const_cast<ImageData*>(image->getParent())); // HACK HACK HACK
            generateBbv(dh);

//             tgt::Texture* batman = _bbv->exportToImageData();
//             ImageData* robin = new ImageData(3, batman->getDimensions(), 1);
//             ImageRepresentationGL::create(robin, batman);
//             data.addData("All glory to the HYPNOTOAD!", robin);

            validate(INVALID_BBV);
        }

        if (_t != 0 && p_useEmptySpaceSkipping.getValue()) {
            // bind
            bbvUnit.activate();
            _t->bind();
            _shader->setIgnoreUniformLocationError(true);
            _shader->setUniform("_bbvTexture", bbvUnit.getUnitNumber());
            _shader->setUniform("_bbvTextureParams._size", tgt::vec3(_t->getDimensions()));
            _shader->setUniform("_bbvTextureParams._sizeRCP", tgt::vec3(1.f) / tgt::vec3(_t->getDimensions()));
            _shader->setUniform("_bbvTextureParams._numChannels", static_cast<int>(1));

            _shader->setUniform("_bbvBrickSize", static_cast<int>(_bbv->getBrickSize()));
            _shader->setUniform("_hasBbv", true);
            _shader->setIgnoreUniformLocationError(false);
        }
        else {
            _shader->setUniform("_hasBbv", false);
        }

        ScopedTypedData<LightSourceData> light(data, p_lightId.getValue());

        if (p_enableShading.getValue() == false || light != nullptr) {
            FramebufferActivationGuard fag(this);
            createAndAttachTexture(GL_RGBA8);
            createAndAttachTexture(GL_RGBA32F);
            createAndAttachTexture(GL_RGBA32F);
            createAndAttachDepthTexture();

            static const GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 , GL_COLOR_ATTACHMENT2 };
            glDrawBuffers(3, buffers);

            if (p_enableShading.getValue() && light != nullptr) {
                light->bind(_shader, "_lightSource");
            }
            if (p_enableShadowing.getValue())
                _shader->setUniform("_shadowIntensity", p_shadowIntensity.getValue());

            glEnable(GL_DEPTH_TEST);
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            QuadRdr.renderQuad();

            glDrawBuffers(1, buffers);
            glDisable(GL_DEPTH_TEST);
            LGL_ERROR;

            data.addData(p_targetImageID.getValue(), new RenderData(_fbo));
        }
        else {
            LDEBUG("Could not load light source from DataContainer.");
        }
    }

    std::string OptimizedRaycaster::generateHeader() const {
        std::string toReturn = RaycastingProcessor::generateHeader();
        if (p_enableShading.getValue())
            toReturn += "#define ENABLE_SHADING\n";
        if (p_enableShadowing.getValue())
            toReturn += "#define ENABLE_SHADOWING\n";
        if (p_enableIntersectionRefinement.getValue())
            toReturn += "#define INTERSECTION_REFINEMENT\n";
        return toReturn;
    }

    void OptimizedRaycaster::updateProperties(DataContainer& dataContainer) {
        RaycastingProcessor::updateProperties(dataContainer);
        p_lightId.setVisible(p_enableShading.getValue());
        p_shadowIntensity.setVisible(p_enableShadowing.getValue());
        validate(INVALID_PROPERTIES);
    }

    void OptimizedRaycaster::generateBbv(DataHandle dh) {
        delete _bbv;
        _bbv = 0;
        delete _t;
        _t = 0;

        if (dh.getData() == 0) {
            return;
        }
        else {
            if (const ImageData* id = dynamic_cast<const ImageData*>(dh.getData())) {
                if (const ImageRepresentationLocal* rep = id->getRepresentation<ImageRepresentationLocal>(true)) {
            	    _bbv = new BinaryBrickedVolume(rep->getParent(), 4);

                    GLubyte* tfBuffer = p_transferFunction.getTF()->getTexture()->downloadTextureToBuffer(GL_RGBA, GL_UNSIGNED_BYTE);
                    size_t tfNumElements = p_transferFunction.getTF()->getTexture()->getDimensions().x;

                    LDEBUG("Start computing brick visibilities...");
                    // parallelly traverse the bricks
                    // have minimum group size 8 to avoid race conditions (every 8 neighbor bricks write to the same byte)!
                    tbb::parallel_for(tbb::blocked_range<size_t>(0, _bbv->getNumBrickIndices(), 8), [&] (const tbb::blocked_range<size_t>& range) {
                        const tgt::vec2& tfIntensityDomain = p_transferFunction.getTF()->getIntensityDomain();

                        for (size_t i = range.begin(); i != range.end(); ++i) {
                            // for each brick, get all corresponding voxels in the reference volume
                            std::vector<tgt::svec3> voxels = _bbv->getAllVoxelsForBrick(i);

                            // traverse the voxels to check whether their intensities are mapped to some opacity
                            for (size_t v = 0; v < voxels.size(); ++v) {
                                // apply same TF lookup as in shader...
                                float intensity = rep->getElementNormalized(voxels[v], 0);
                                if (intensity >= tfIntensityDomain.x || intensity <= tfIntensityDomain.y) {
                                    float mappedIntensity = (intensity - tfIntensityDomain.x) / (tfIntensityDomain.y - tfIntensityDomain.x);
                                    tgtAssert(mappedIntensity >= 0.f && mappedIntensity <= 1.f, "Mapped intensity out of bounds!");

                                    // ...but with nearest neighbour interpolation
                                    size_t scaled = static_cast<size_t>(mappedIntensity * static_cast<float>(tfNumElements - 1));
                                    tgtAssert(scaled < tfNumElements, "Somebody did the math wrong...");
                                    GLubyte opacity = tfBuffer[(4 * (scaled)) + 3];
                                    if (opacity != 0) {
                                        _bbv->setValueForIndex(i, true);
                                        break;
                                    }
                                }
                            }
                        }
                    });

                    LDEBUG("...finished computing brick visibilities.");

                    // export to texture:
                    _t = _bbv->exportToImageData();
                }
                else {
                    LERROR("Could not convert to a local representation.");
                }
            }
            else {
                tgtAssert(false, "The data type in the given DataHandle is WRONG!");
            }
        }
    }


}
