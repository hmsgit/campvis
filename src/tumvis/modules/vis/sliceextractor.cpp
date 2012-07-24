#include "sliceextractor.h"
#include "tgt/logmanager.h"
#include "tgt/quadrenderer.h"
#include "tgt/textureunit.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagedatagl.h"
#include "core/datastructures/imagedatarendertarget.h"
#include "core/datastructures/imagedataconverter.h"

namespace TUMVis {
    const std::string SliceExtractor::loggerCat_ = "TUMVis.modules.vis.SliceExtractor";

    SliceExtractor::SliceExtractor(GenericProperty<tgt::ivec2>& canvasSize)
        : VisualizationProcessor(canvasSize)
        , _sourceImageID("sourceImageID", "Input Image", "")
        , _targetImageID("targetImageID", "Output Image", "")
        , _sliceNumber("sliceNumber", "Slice Number", 0, 0, 0)
        , _shader(0)
    {
        _properties.addProperty(&_sourceImageID);
        _properties.addProperty(&_targetImageID);
        _properties.addProperty(&_sliceNumber);
        _sliceNumber.addObserver(this);
    }

    SliceExtractor::~SliceExtractor() {

    }

    void SliceExtractor::init() {
        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/vis/sliceextractor.frag", "", false);
    }

    void SliceExtractor::deinit() {
        ShdrMgr.dispose(_shader);
    }

    void SliceExtractor::process(DataContainer& data) {
        const ImageDataLocal* img = data.getTypedData<ImageDataLocal>(_sourceImageID.getValue());

        if (img != 0) {
            if (img->getDimensionality() == 3) {
                updateProperties(img);
                const tgt::svec3& imgSize = img->getSize();
                ImageDataLocal* slice = img->getSubImage(tgt::svec3(0, 0, _sliceNumber.getValue()), tgt::svec3(imgSize.x-1, imgSize.y-1, _sliceNumber.getValue()));
                ImageDataGL* glData = ImageDataConverter::tryConvert<ImageDataGL>(slice);
                ImageDataRenderTarget* rt = new ImageDataRenderTarget(tgt::svec3(_renderTargetSize.getValue(), 1));

                _shader->activate();
                tgt::TextureUnit inputUnit;
                glData->bind(_shader, inputUnit);

                rt->activate();
                glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
                tgt::QuadRenderer::renderQuad();
                rt->deactivate();

                _shader->deactivate();

                data.addData(_targetImageID.getValue(), rt);
                delete slice;
                delete glData;
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

    void SliceExtractor::updateProperties(const ImageData* img) {
        const tgt::svec3& imgSize = img->getSize();
        if (_sliceNumber.getMaxValue() != imgSize.z - 1){
            _sliceNumber.setMaxValue(imgSize.z - 1);
        }
    }

}
