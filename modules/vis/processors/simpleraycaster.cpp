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
#include "core/datastructures/imagedatarendertarget.h"
#include "core/pipeline/processordecoratorshading.h"

namespace campvis {
    const std::string SimpleRaycaster::loggerCat_ = "CAMPVis.modules.vis.SimpleRaycaster";

    SimpleRaycaster::SimpleRaycaster(GenericProperty<tgt::ivec2>& canvasSize)
        : RaycastingProcessor(canvasSize, "modules/vis/glsl/simpleraycaster.frag", true)
        , _targetImageID("targetImageID", "Output Image", "", DataNameProperty::WRITE)
        , _enableShadowing("EnableShadowing", "Enable Hard Shadows", false, InvalidationLevel::INVALID_SHADER)
        , _shadowIntensity("ShadowIntensity", "Shadow Intensity", .5f, .0f, 1.f)
    {
        addDecorator(new ProcessorDecoratorShading());

        addProperty(&_targetImageID);
        addProperty(&_enableShadowing);
        addProperty(&_shadowIntensity);
        decoratePropertyCollection(this);
    }

    SimpleRaycaster::~SimpleRaycaster() {

    }

    void SimpleRaycaster::processImpl(DataContainer& data) {
        ImageDataRenderTarget* output = new ImageDataRenderTarget(tgt::svec3(_renderTargetSize.getValue(), 1));
        output->createAndAttachTexture(GL_RGBA32F);
        output->createAndAttachTexture(GL_RGBA32F);
        output->activate();

        GLenum buffers[] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1 , GL_COLOR_ATTACHMENT2 };
        glDrawBuffers(3, buffers);

        if (_enableShadowing.getValue())
            _shader->setUniform("_shadowIntensity", _shadowIntensity.getValue());

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        glDepthFunc(GL_ALWAYS);
        QuadRdr.renderQuad();
        LGL_ERROR;

        output->deactivate();
        data.addData(_targetImageID.getValue(), output);
        _targetImageID.issueWrite();
    }

    std::string SimpleRaycaster::generateHeader() const {
        std::string toReturn = RaycastingProcessor::generateHeader();
        if (_enableShadowing.getValue())
            toReturn += "#define ENABLE_SHADOWING\n";
        return toReturn;
    }

}
