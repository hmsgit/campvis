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

#include "simpleraycaster.h"

#include "core/tools/quadrenderer.h"
#include "core/datastructures/renderdata.h"
#include "core/pipeline/processordecoratorshading.h"

#include <tbb/tbb.h>

namespace campvis {
    const std::string SimpleRaycaster::loggerCat_ = "CAMPVis.modules.vis.SimpleRaycaster";

    SimpleRaycaster::SimpleRaycaster(IVec2Property* viewportSizeProp)
        : RaycastingProcessor(viewportSizeProp, "modules/vis/glsl/simpleraycaster.frag", true)
        , p_targetImageID("targetImageID", "Output Image", "", DataNameProperty::WRITE)
        , p_enableShadowing("EnableShadowing", "Enable Hard Shadows (Expensive!)", false, AbstractProcessor::INVALID_RESULT | AbstractProcessor::INVALID_SHADER | AbstractProcessor::INVALID_PROPERTIES)
        , p_shadowIntensity("ShadowIntensity", "Shadow Intensity", .5f, .0f, 1.f)
        , p_enableAdaptiveStepsize("EnableAdaptiveStepSize", "Enable Adaptive Step Size", true, AbstractProcessor::INVALID_RESULT | AbstractProcessor::INVALID_SHADER)
        , p_useEmptySpaceSkipping("EnableEmptySpaceSkipping", "Enable Empty Space Skipping", false, AbstractProcessor::INVALID_RESULT | INVALID_BBV)
        , _bbv(0)
        , _t(0)
    {
        addDecorator(new ProcessorDecoratorShading());

        addProperty(&p_targetImageID);
        addProperty(&p_enableAdaptiveStepsize);
        addProperty(&p_useEmptySpaceSkipping);

        addProperty(&p_enableShadowing);
        addProperty(&p_shadowIntensity);
        p_shadowIntensity.setVisible(false);

        decoratePropertyCollection(this);
    }

    SimpleRaycaster::~SimpleRaycaster() {
        delete _bbv;
        delete _t;
    }

    void SimpleRaycaster::init() {
        RaycastingProcessor::init();
    }

    void SimpleRaycaster::deinit() {
        RaycastingProcessor::deinit();
    }

    void SimpleRaycaster::processImpl(DataContainer& data, ImageRepresentationGL::ScopedRepresentation& image) {
        tgt::TextureUnit bbvUnit;

        if (getInvalidationLevel() & INVALID_BBV) {
            DataHandle dh = DataHandle(const_cast<ImageData*>(image->getParent())); // HACK HACK HACK
            generateBbv(dh);
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

            _shader->setUniform("_bbvBrickSize", 2);
            _shader->setUniform("_hasBbv", true);
            _shader->setIgnoreUniformLocationError(false);
        }
        else {
            _shader->setUniform("_hasBbv", false);
        }

        FramebufferActivationGuard fag(this);
        createAndAttachTexture(GL_RGBA8);
//         createAndAttachTexture(GL_RGBA32F);
//         createAndAttachTexture(GL_RGBA32F);
        createAndAttachDepthTexture();

//         static const GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 , GL_COLOR_ATTACHMENT2 };
//         glDrawBuffers(3, buffers);

        if (p_enableShadowing.getValue())
            _shader->setUniform("_shadowIntensity", p_shadowIntensity.getValue());

        glEnable(GL_DEPTH_TEST);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        QuadRdr.renderQuad();
        glDisable(GL_DEPTH_TEST);
        LGL_ERROR;

        data.addData(p_targetImageID.getValue(), new RenderData(_fbo));
    }

    std::string SimpleRaycaster::generateHeader() const {
        std::string toReturn = RaycastingProcessor::generateHeader();
        if (p_enableShadowing.getValue())
            toReturn += "#define ENABLE_SHADOWING\n";
        if (p_enableAdaptiveStepsize.getValue())
            toReturn += "#define ENABLE_ADAPTIVE_STEPSIZE\n";
        return toReturn;
    }

    void SimpleRaycaster::updateProperties() {
        p_shadowIntensity.setVisible(p_enableShadowing.getValue());
        validate(AbstractProcessor::INVALID_PROPERTIES);
    }

    void SimpleRaycaster::generateBbv(DataHandle dh) {
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
            	    _bbv = new BinaryBrickedVolume(rep->getParent(), 2);

                    GLubyte* tfBuffer = p_transferFunction.getTF()->getTexture()->downloadTextureToBuffer(GL_RGBA, GL_UNSIGNED_BYTE);
                    size_t tfNumElements = p_transferFunction.getTF()->getTexture()->getDimensions().x;

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
