// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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

#include "virtualmirrorcombine.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/imagerepresentationrendertarget.h"


#include "core/classification/simpletransferfunction.h"

#include "core/tools/quadrenderer.h"

namespace campvis {
    const std::string VirtualMirrorCombine::loggerCat_ = "CAMPVis.modules.vis.VirtualMirrorCombine";

    VirtualMirrorCombine::VirtualMirrorCombine(GenericProperty<tgt::ivec2>& canvasSize)
        : VisualizationProcessor(canvasSize)
        , p_normalImageID("normalImageID", "Normal DVR Input Image", "", DataNameProperty::READ)
        , p_mirrorImageID("mirrorImageID", "Mirror DVR Input Image", "", DataNameProperty::READ)
        , p_mirrorRenderID("mirrorRenderID", "Rendered Mirror Input Image", "", DataNameProperty::READ)
        , p_targetImageID("targetImageID", "Output Image", "", DataNameProperty::WRITE)
        , _shader(0)
    {
        addProperty(&p_normalImageID);
        addProperty(&p_mirrorImageID);
        addProperty(&p_mirrorRenderID);
        addProperty(&p_targetImageID);

        addDecorator(new ProcessorDecoratorBackground());
        
        decoratePropertyCollection(this);
    }

    VirtualMirrorCombine::~VirtualMirrorCombine() {

    }

    void VirtualMirrorCombine::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/vis/glsl/virtualmirrorcombine.frag", "", false);
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");
    }

    void VirtualMirrorCombine::deinit() {
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
    }

    void VirtualMirrorCombine::process(DataContainer& data) {
        ImageRepresentationRenderTarget::ScopedRepresentation normalImage(data, p_normalImageID.getValue());
        ImageRepresentationRenderTarget::ScopedRepresentation mirrorImage(data, p_mirrorImageID.getValue());
        ImageRepresentationRenderTarget::ScopedRepresentation mirrorRendered(data, p_mirrorRenderID.getValue());

        if (normalImage != 0 && mirrorImage != 0 && mirrorRendered != 0) {
            std::pair<ImageData*, ImageRepresentationRenderTarget*> rt = ImageRepresentationRenderTarget::createWithImageData(_renderTargetSize.getValue());
            glPushAttrib(GL_ALL_ATTRIB_BITS);
            glEnable(GL_DEPTH_TEST);
            glDepthFunc(GL_ALWAYS);

            _shader->activate();
            tgt::TextureUnit normalColorUnit, normalDepthUnit, mirrorColorUnit, mirrorDepthUnit, mirrorRenderedDepthUnit;

            normalImage->bind(_shader, &normalColorUnit, &normalDepthUnit, "_normalColor", "_normalDepth");
            mirrorImage->bind(_shader, &mirrorColorUnit, &mirrorDepthUnit, "_mirrorColor", "_mirrorDepth");
            mirrorRendered->bind(_shader, 0, &mirrorRenderedDepthUnit, "", "_mirrorRenderedDepth");

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

            data.addData(p_targetImageID.getValue(), rt.first);
            p_targetImageID.issueWrite();
        }
        else {
            LERROR("No suitable input images found.");
        }

        _invalidationLevel.setValid();
    }

}

