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

#include "quadview.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/imagerepresentationrendertarget.h"


#include "core/classification/simpletransferfunction.h"

#include "core/tools/quadrenderer.h"

namespace campvis {
    const std::string QuadView::loggerCat_ = "CAMPVis.modules.vis.QuadView";

    QuadView::QuadView(IVec2Property& canvasSize)
        : VisualizationProcessor(canvasSize)
        , p_inputImage1("InputImage1", "First Input Image", "", DataNameProperty::READ)
        , p_inputImage2("InputImage2", "Second Input Image", "", DataNameProperty::READ)
        , p_inputImage3("InputImage3", "Third Input Image", "", DataNameProperty::READ)
        , p_inputImage4("InputImage4", "Fourth Input Image", "", DataNameProperty::READ)
        , p_outputImage("OutputImage", "Output Image", "dd.output", DataNameProperty::WRITE)
        , _shader(0)
    {
        addProperty(&p_inputImage1);
        addProperty(&p_inputImage2);
        addProperty(&p_inputImage3);
        addProperty(&p_inputImage4);
        addProperty(&p_outputImage);
    }

    QuadView::~QuadView() {

    }

    void QuadView::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/vis/glsl/quadview.frag", "", false);
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void QuadView::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void QuadView::process(DataContainer& data) {
        ImageRepresentationRenderTarget::ScopedRepresentation inputImage1(data, p_inputImage1.getValue());
        ImageRepresentationRenderTarget::ScopedRepresentation inputImage2(data, p_inputImage2.getValue());
        ImageRepresentationRenderTarget::ScopedRepresentation inputImage3(data, p_inputImage3.getValue());
        ImageRepresentationRenderTarget::ScopedRepresentation inputImage4(data, p_inputImage4.getValue());

        if (inputImage1 == 0 && inputImage2 == 0 && inputImage3 == 0 && inputImage4 == 0)
            return;

        std::pair<ImageData*, ImageRepresentationRenderTarget*> outputTarget = ImageRepresentationRenderTarget::createWithImageData(_renderTargetSize.getValue());
        tgt::TextureUnit colorUnit, depthUnit;
        _shader->activate();
        _shader->setUniform("_modelMatrix", tgt::mat4::createScale(tgt::vec3(.5f, .5f, .5f)));
        outputTarget.second->activate();
        LGL_ERROR;
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (inputImage1 != 0) {
            inputImage1->bind(_shader, colorUnit, depthUnit);
            _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(-.5f, .5f, 0.f)));
            QuadRdr.renderQuad();
        }
        if (inputImage2 != 0) {
            inputImage2->bind(_shader, colorUnit, depthUnit);
            _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(.5f, .5f, 0.f)));
            QuadRdr.renderQuad();
        }
        if (inputImage3 != 0) {
            inputImage3->bind(_shader, colorUnit, depthUnit);
            _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(-.5f, -.5f, 0.f)));
            QuadRdr.renderQuad();
        }
        if (inputImage4 != 0) {
            inputImage4->bind(_shader, colorUnit, depthUnit);
            _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(.5f, -.5f, 0.f)));
            QuadRdr.renderQuad();
        }

        outputTarget.second->deactivate();
        _shader->deactivate();
        tgt::TextureUnit::setZeroUnit();
        LGL_ERROR;

        data.addData(p_outputImage.getValue(), outputTarget.first);
        p_outputImage.issueWrite();
        validate(INVALID_RESULT);
    }

}

