#include "slicevis.h"

#include "tgt/glcontext.h"
#include "core/datastructures/imagedataconverter.h"

namespace TUMVis {

    SliceVis::SliceVis(tgt::GLCanvas* canvas)
        : VisualizationPipeline(canvas)
        , _imageReader()
        , _sliceExtractor(_renderTargetSize)
        , _wheelHandler(&_sliceExtractor._sliceNumber)
    {
        _processors.push_back(&_imageReader);
        _processors.push_back(&_sliceExtractor);
        _eventHandlers.push_back(&_wheelHandler);
    }

    SliceVis::~SliceVis() {
    }

    void SliceVis::init() {
        VisualizationPipeline::init();

        _imageReader._url.setValue("D:\\Medical Data\\smallHeart.mhd");
        _imageReader._targetImageID.setValue("reader.output");

        _sliceExtractor._sourceImageID.setValue("se.input");
        _sliceExtractor._sliceNumber.setValue(0);
        // TODO: replace this hardcoded domain by automatically determined from image min/max values
        _sliceExtractor._transferFunction.getTF()->setIntensityDomain(tgt::vec2(0, 0.05f));

        _renderTargetID.setValue("renderTarget");
        _renderTargetID.addSharedProperty(&(_sliceExtractor._targetImageID));

        _imageReader.addObserver(this);
        _sliceExtractor.addObserver(this);
    }

    void SliceVis::execute() {
        {
            tbb::spin_mutex::scoped_lock lock(_localMutex);
            _invalidationLevel.setValid();
            // TODO:    think whether we want to lock all processors already here.
        }
        if (! _imageReader.getInvalidationLevel().isValid()) {
            executeProcessor(_imageReader);

            // convert data
            const ImageData* img = _data.getTypedData<ImageData>("reader.output");
            ImageDataLocal* local = ImageDataConverter::tryConvert<ImageDataLocal>(img);
            if (local != 0) {
                _data.addData("se.input", local);
            }
        }
        if (! _sliceExtractor.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(_sliceExtractor);
        }
    }

    void SliceVis::keyEvent(tgt::KeyEvent* e) {
        if (e->pressed()) {
            switch (e->keyCode()) {
                case tgt::KeyEvent::K_UP:
                    _sliceExtractor._sliceNumber.increment();
                    break;
                case tgt::KeyEvent::K_DOWN:
                    _sliceExtractor._sliceNumber.decrement();
                    break;
            }
        }
    }

}