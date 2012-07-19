#include "slicevis.h"

#include "core/datastructures/imagedataconverter.h"

namespace TUMVis {

    SliceVis::SliceVis()
        : VisualizationPipeline()
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

        _renderTargetID.setValue("renderTarget");
        _renderTargetID.addSharedProperty(&(_sliceExtractor._targetImageID));

        _imageReader.addObserver(this);
        _sliceExtractor.addObserver(this);
        execute();
    }

    void SliceVis::execute() {
        {
            tbb::spin_mutex::scoped_lock lock(_localMutex);
            _invalidationLevel.setValid();
        }
        if (! _imageReader.getInvalidationLevel().isValid()) {
            _imageReader.process(_data);

            // convert data
            const ImageData* img = _data.getTypedData<ImageData>("reader.output");
            ImageDataLocal* local = ImageDataConverter::tryConvert<ImageDataLocal>(img);
            if (local != 0) {
                _data.addData("se.input", local);
            }
        }
        if (! _sliceExtractor.getInvalidationLevel().isValid()) {
            _sliceExtractor.process(_data);
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