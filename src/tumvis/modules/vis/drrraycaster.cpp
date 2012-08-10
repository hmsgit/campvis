#include "drrraycaster.h"

#include "tgt/quadrenderer.h"
#include "core/datastructures/imagedatarendertarget.h"

namespace TUMVis {
    const std::string DRRRaycaster::loggerCat_ = "TUMVis.modules.vis.DRRRaycaster";

    DRRRaycaster::DRRRaycaster(GenericProperty<tgt::ivec2>& canvasSize)
        : RaycastingProcessor(canvasSize, "modules/vis/drrraycaster.frag", true) // FIXME: the last parameter should be false, but it results in a wrong rendering
        , _targetImageID("targetImageID", "Output Image", "")
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
        tgt::QuadRenderer::renderQuad();
        LGL_ERROR;

        rt->deactivate();
        data.addData(_targetImageID.getValue(), rt);
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
