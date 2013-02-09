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

#include "rendertargetcompositor.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/imagerepresentationrendertarget.h"


#include "core/classification/simpletransferfunction.h"

#include "core/tools/quadrenderer.h"

namespace campvis {

    static const GenericOption<CompositingMode> compositingOptions[5] = {
        GenericOption<CompositingMode>("first", "Only First", CompositingModeFirst),
        GenericOption<CompositingMode>("second", "Only Second", CompositingModeSecond),
        GenericOption<CompositingMode>("alpha", "Alpha Blending", CompositingModeAlpha),
        GenericOption<CompositingMode>("diff", "Difference", CompositingModeDifference),
        GenericOption<CompositingMode>("depth", "Depth Test", CompositingModeDepth)
    };

    const std::string RenderTargetCompositor::loggerCat_ = "CAMPVis.modules.vis.RenderTargetCompositor";

    RenderTargetCompositor::RenderTargetCompositor(GenericProperty<tgt::ivec2>& canvasSize)
        : VisualizationProcessor(canvasSize)
        , p_firstImageId("FirstImageId", "First Input Image", "", DataNameProperty::READ)
        , p_secondImageId("SecondImageId", "Second Input Image", "", DataNameProperty::READ)
        , p_targetImageId("TargetImageId", "Output Image", "", DataNameProperty::WRITE)
        , p_compositingMethod("CompositingMethod", "Compositing Method", compositingOptions, 5)
        , p_alphaValue("AlphaValue", "AlphaValue", .5f, 0.f, 1.f)
        , _shader(0)
    {
        addProperty(&p_firstImageId);
        addProperty(&p_secondImageId);
        addProperty(&p_targetImageId);
        addProperty(&p_compositingMethod);
        addProperty(&p_alphaValue);

        addDecorator(new ProcessorDecoratorBackground());
        
        decoratePropertyCollection(this);
    }

    RenderTargetCompositor::~RenderTargetCompositor() {

    }

    void RenderTargetCompositor::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/vis/glsl/rendertargetcompositor.frag", "", false);
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void RenderTargetCompositor::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void RenderTargetCompositor::process(DataContainer& data) {
        ImageRepresentationRenderTarget::ScopedRepresentation firstImage(data, p_firstImageId.getValue());
        ImageRepresentationRenderTarget::ScopedRepresentation secondImage(data, p_secondImageId.getValue());

        if (firstImage != 0 && secondImage != 0 ) {
            std::pair<ImageData*, ImageRepresentationRenderTarget*> rt = ImageRepresentationRenderTarget::createWithImageData(_renderTargetSize.getValue());
            glPushAttrib(GL_ALL_ATTRIB_BITS);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_ALWAYS);

            _shader->activate();
            tgt::TextureUnit firstColorUnit, firstDepthUnit, secondColorUnit, secondDepthUnit;

            firstImage->bind(_shader, firstColorUnit, firstDepthUnit, "_firstColor", "_firstDepth", "_firstTexParams");
            secondImage->bind(_shader, secondColorUnit, secondDepthUnit, "_secondColor", "_secondDepth", "_secondTexParams");
            _shader->setUniform("_compositingMethod", p_compositingMethod.getOptionValue());
            _shader->setUniform("_alpha", p_alphaValue.getValue());

            decorateRenderProlog(data, _shader);

            rt.second->activate();
            LGL_ERROR;
            glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
            QuadRdr.renderQuad();
            rt.second->deactivate();

            _shader->deactivate();
            tgt::TextureUnit::setZeroUnit();
            glPopAttrib();
            LGL_ERROR;

            data.addData(p_targetImageId.getValue(), rt.first);
            p_targetImageId.issueWrite();
        }
        else {
            LERROR("No suitable input images found.");
        }

        applyInvalidationLevel(InvalidationLevel::VALID);
    }

}

