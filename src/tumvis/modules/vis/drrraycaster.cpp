#include "drrraycaster.h"

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
    const std::string DRRRaycaster::loggerCat_ = "TUMVis.modules.vis.DRRRaycaster";

    DRRRaycaster::DRRRaycaster(GenericProperty<tgt::ivec2>& canvasSize)
        : VisualizationProcessor(canvasSize)
        , _sourceImageID("sourceImageID", "Input Image", "")
        , _entryImageID("entryImageID", "Output Entry Points Image", "")
        , _exitImageID("exitImageID", "Output Exit Points Image", "")
        , _targetImageID("targetImageID", "Output Image", "")
        , _transferFunction("transferFunction", "Transfer Function", new SimpleTransferFunction(256))
        , _samplingStepSize("samplingStepSize", "Sampling Step Size", .5f, 0.01f, 2.f)
        , _shift("shift", "Normalization Shift", 0.f, -10.f, 10.f)
        , _scale("scale", "Normalization Scale", 1.f, 0.f, 1000.f)
        , _invertMapping("invertMapping", "Invert Mapping", false, InvalidationLevel::INVALID_RESULT | InvalidationLevel::INVALID_SHADER)
        , _shader(0)
    {
        addProperty(&_sourceImageID);
        addProperty(&_entryImageID);
        addProperty(&_exitImageID);
        addProperty(&_targetImageID);
        addProperty(&_transferFunction);
        addProperty(&_samplingStepSize);
        addProperty(&_shift);
        addProperty(&_scale);
        addProperty(&_invertMapping);
    }

    DRRRaycaster::~DRRRaycaster() {

    }

    void DRRRaycaster::init() {
        VisualizationProcessor::init();
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/vis/drrraycaster.frag", "", false);
    }

    void DRRRaycaster::deinit() {
        ShdrMgr.dispose(_shader);
        _shader = 0;
        VisualizationProcessor::deinit();
    }

    void DRRRaycaster::process(DataContainer& data) {
        DataContainer::ScopedTypedData<ImageDataGL> img(data, _sourceImageID.getValue());
        DataContainer::ScopedTypedData<ImageDataRenderTarget> entryPoints(data, _entryImageID.getValue());
        DataContainer::ScopedTypedData<ImageDataRenderTarget> exitPoints(data, _exitImageID.getValue());

        if (img != 0 && entryPoints != 0 && exitPoints != 0) {
            if (img->getDimensionality() == 3) {
                if (_invalidationLevel.isInvalidShader()) {
                    _shader->setHeaders(generateHeader());
                    _shader->rebuild();
                }

                ImageDataRenderTarget* output = new ImageDataRenderTarget(tgt::svec3(_renderTargetSize.getValue(), 1));

                _shader->activate();
                _shader->setUniform("_samplingStepSize", _samplingStepSize.getValue());
                _shader->setUniform("_shift", _shift.getValue());
                _shader->setUniform("_scale", _scale.getValue());

                tgt::TextureUnit volumeUnit, entryUnit, exitUnit, tfUnit;
                img->bind(_shader, volumeUnit, "_volume");
                entryPoints->bind(_shader, &entryUnit, 0, "_entryPoints", "", "_entryParameters");
                exitPoints->bind(_shader, &exitUnit, 0, "_exitPoints", "", "_exitParameters");
                _transferFunction.getTF()->bind(_shader, tfUnit);

                output->activate();
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                tgt::QuadRenderer::renderQuad();
                output->deactivate();

                _shader->deactivate();
                tgt::TextureUnit::setZeroUnit();

                data.addData(_targetImageID.getValue(), output);
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

    std::string DRRRaycaster::generateHeader() const {
        std::string toReturn;

        if (_invertMapping.getValue())
            toReturn += "#define DRR_INVERT 1\n";
//         if (depthMapping_.get())
//             header +="#define DEPTH_MAPPING 1\n";

        return toReturn;
    }

}
