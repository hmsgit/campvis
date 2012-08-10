#include "simpleraycaster.h"

#include "tgt/quadrenderer.h"
#include "core/datastructures/imagedatarendertarget.h"

namespace TUMVis {
    const std::string SimpleRaycaster::loggerCat_ = "TUMVis.modules.vis.SimpleRaycaster";

    SimpleRaycaster::SimpleRaycaster(GenericProperty<tgt::ivec2>& canvasSize)
        : RaycastingProcessor(canvasSize, "modules/vis/simpleraycaster.frag", true)
        , _targetImageID("targetImageID", "Output Image", "")
    {
        addProperty(&_targetImageID);
    }

    SimpleRaycaster::~SimpleRaycaster() {

    }

    void SimpleRaycaster::processImpl(DataContainer& data) {
        ImageDataRenderTarget* rt = new ImageDataRenderTarget(tgt::svec3(_renderTargetSize.getValue(), 1));
        rt->activate();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        tgt::QuadRenderer::renderQuad();
        LGL_ERROR;

        rt->deactivate();
        data.addData(_targetImageID.getValue(), rt);
    }
}
