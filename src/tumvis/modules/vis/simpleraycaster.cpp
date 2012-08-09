#include "simpleraycaster.h"

#include "tgt/logmanager.h"
#include "tgt/quadrenderer.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagedatagl.h"
#include "core/datastructures/imagedatarendertarget.h"
#include "core/datastructures/imagedataconverter.h"

#include "core/classification/simpletransferfunction.h"

namespace TUMVis {
    const std::string SimpleRaycaster::loggerCat_ = "TUMVis.modules.vis.SimpleRaycaster";

    SimpleRaycaster::SimpleRaycaster(GenericProperty<tgt::ivec2>& canvasSize)
        : VisualizationProcessor(canvasSize)
        , _sourceImageID("sourceImageID", "Input Image", "")
        , _entryImageID("entryImageID", "Output Entry Points Image", "")
        , _exitImageID("exitImageID", "Output Exit Points Image", "")
        , _targetImageID("targetImageID", "Output Image", "")
        , _transferFunction("transferFunction", "Transfer Function", new SimpleTransferFunction(256))
        , _samplingStepSize("samplingStepSize", "Sampling Step Size", .1f, 0.001f, 1.f)
        , _jitterEntryPoints("jitterEntryPoints", "Jitter Entry Points", true)
        , _shader(0)
    {
        addProperty(&_sourceImageID);
        addProperty(&_entryImageID);
        addProperty(&_exitImageID);
        addProperty(&_targetImageID);
        addProperty(&_transferFunction);
        addProperty(&_samplingStepSize);
        addProperty(&_jitterEntryPoints);
    }

    SimpleRaycaster::~SimpleRaycaster() {

    }

    void SimpleRaycaster::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/vis/simpleraycaster.frag", "", false);
        _shader->setHeaders(generateHeader());
        _shader->rebuild();
    }

    void SimpleRaycaster::deinit() {
        ShdrMgr.dispose(_shader);
        _shader = 0;
        VisualizationProcessor::deinit();
    }

    void SimpleRaycaster::process(DataContainer& data) {
        DataContainer::ScopedTypedData<ImageDataGL> img(data, _sourceImageID.getValue());
        DataContainer::ScopedTypedData<ImageDataRenderTarget> entryPoints(data, _entryImageID.getValue());
        DataContainer::ScopedTypedData<ImageDataRenderTarget> exitPoints(data, _exitImageID.getValue());

        if (img != 0 && entryPoints != 0 && exitPoints != 0) {
            if (img->getDimensionality() == 3) {
                if (_invalidationLevel.isInvalidShader()) {
                    _shader->setHeaders(generateHeader());
                    _shader->rebuild();
                }

                ImageDataRenderTarget* rt = new ImageDataRenderTarget(tgt::svec3(_renderTargetSize.getValue(), 1));

                glPushAttrib(GL_ALL_ATTRIB_BITS);
                _shader->activate();
                _shader->setUniform("_viewportSizeRCP", 1.f / tgt::vec2(_renderTargetSize.getValue()));
                _shader->setUniform("_jitterEntryPoints", _jitterEntryPoints.getValue());
                _shader->setUniform("_samplingStepSize", _samplingStepSize.getValue());

                tgt::TextureUnit volumeUnit, entryUnit, entryUnitDepth, exitUnit, exitUnitDepth, tfUnit;
                img->bind(_shader, volumeUnit, "_volume");
                entryPoints->bind(_shader, &entryUnit, &entryUnitDepth, "_entryPoints", "_entryPointsDepth");
                exitPoints->bind(_shader, &exitUnit, &exitUnitDepth, "_exitPoints", "_exitPointsDepth");
                _transferFunction.getTF()->bind(_shader, tfUnit);

                rt->activate();

                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                LGL_ERROR;
                tgt::QuadRenderer::renderQuad();
                LGL_ERROR;
                rt->deactivate();

                _shader->deactivate();
                tgt::TextureUnit::setZeroUnit();
                glPopAttrib();

                data.addData(_targetImageID.getValue(), rt);
            }
            else {
                LERROR("Input image must have dimensionality of 3.");
            }
        }
        else {
            LERROR("No suitable input image found.");
        }

        _invalidationLevel.setValid();
    }

    std::string SimpleRaycaster::generateHeader() const {
        std::string toReturn;

        return toReturn;
    }

}
