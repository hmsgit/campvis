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

#include "drrraycaster.h"

#include "core/tools/quadrenderer.h"
#include "core/datastructures/imagedatarendertarget.h"

namespace campvis {
    const std::string DRRRaycaster::loggerCat_ = "CAMPVis.modules.vis.DRRRaycaster";

    DRRRaycaster::DRRRaycaster(GenericProperty<tgt::ivec2>& canvasSize)
        : RaycastingProcessor(canvasSize, "modules/vis/drrraycaster.frag", false)
        , _targetImageID("targetImageID", "Output Image", "", DataNameProperty::WRITE)
        , _shift("shift", "Normalization Shift", 0.f, -10.f, 10.f)
        , _scale("scale", "Normalization Scale", 1.f, 0.f, 1000.f)
        , _invertMapping("invertMapping", "Invert Mapping", false, InvalidationLevel::INVALID_RESULT | InvalidationLevel::INVALID_SHADER)
    {
        addProperty(&_targetImageID);
        addProperty(&_shift);
        addProperty(&_scale);
        addProperty(&_invertMapping);
    }

    DRRRaycaster::~DRRRaycaster() {

    }

    void DRRRaycaster::processImpl(DataContainer& data) {
        _shader->setUniform("_shift", _shift.getValue());
        _shader->setUniform("_scale", _scale.getValue());

        ImageDataRenderTarget* rt = new ImageDataRenderTarget(tgt::svec3(_renderTargetSize.getValue(), 1));
        rt->activate();

        if (_invertMapping.getValue())
            glClearColor(0.f, 0.f, 0.f, 1.f);
        else
            glClearColor(1.f, 1.f, 1.f, 1.f);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        QuadRdr.renderQuad();
        LGL_ERROR;

        rt->deactivate();
        data.addData(_targetImageID.getValue(), rt);
        _targetImageID.issueWrite();
    }

    std::string DRRRaycaster::generateHeader() const {
        std::string toReturn;

        if (_invertMapping.getValue())
            toReturn += "#define DRR_INVERT 1\n";
        //         if (depthMapping_.get())
        //             header +="#define DEPTH_MAPPING 1\n";

        return toReturn;
    }
}
