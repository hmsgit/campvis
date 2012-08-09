#include "dvrvis.h"

#include "tgt/event/keyevent.h"
#include "tgt/glcontext.h"
#include "core/datastructures/imagedataconverter.h"

namespace TUMVis {

    DVRVis::DVRVis()
        : VisualizationPipeline()
        , _imageReader()
        , _eepGenerator(_renderTargetSize)
        , _drrraycater(_renderTargetSize)
        , _simpleRaycaster(_renderTargetSize)
        , _trackballEH(0)
    {
        _trackballEH = new TrackballNavigationEventHandler(&_eepGenerator._camera, _renderTargetSize.getValue());
        _eventHandlers.push_back(_trackballEH);

        _processors.push_back(&_imageReader);
        _processors.push_back(&_eepGenerator);
        _processors.push_back(&_drrraycater);
        _processors.push_back(&_simpleRaycaster);
    }

    DVRVis::~DVRVis() {
        delete _trackballEH;
    }

    void DVRVis::init() {
        VisualizationPipeline::init();

        _imageReader._url.setValue("D:\\Medical Data\\smallHeart.mhd");
        _imageReader._targetImageID.setValue("reader.output");

        _eepGenerator._entryImageID.addSharedProperty(&_drrraycater._entryImageID);
        _eepGenerator._entryImageID.addSharedProperty(&_simpleRaycaster._entryImageID);
        _eepGenerator._exitImageID.addSharedProperty(&_drrraycater._exitImageID);
        _eepGenerator._exitImageID.addSharedProperty(&_simpleRaycaster._exitImageID);

        _drrraycater._targetImageID.setValue("drr.output");
        _drrraycater._sourceImageID.setValue("eep.input");

        _simpleRaycaster._targetImageID.setValue("dvr.output");
        _simpleRaycaster._sourceImageID.setValue("eep.input");

        _eepGenerator._sourceImageID.setValue("eep.input");
        _eepGenerator._entryImageID.setValue("eep.entry");
        _eepGenerator._exitImageID.setValue("eep.exit");

        _renderTargetID.setValue("drr.output");

        _imageReader.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);
        _eepGenerator.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);
        _drrraycater.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);
        _simpleRaycaster.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);
    }

    void DVRVis::execute() {
        {
            tbb::spin_mutex::scoped_lock lock(_localMutex);
            _invalidationLevel.setValid();
            // TODO:    think whether we want to lock all processors already here.
        }
        if (! _imageReader.getInvalidationLevel().isValid()) {
            executeProcessor(_imageReader);

            // convert data
            DataContainer::ScopedTypedData<ImageData> img(_data, "reader.output");
            {
                tgt::GLContextScopedLock lock(_canvas->getContext());
                ImageDataGL* gl = ImageDataConverter::tryConvert<ImageDataGL>(img);
                if (gl != 0) {
                    _data.addData("eep.input", gl);
                }

                tgt::Bounds volumeExtent = img->getWorldBounds();
                tgt::vec3 pos = volumeExtent.center() - tgt::vec3(0, 0, tgt::length(volumeExtent.diagonal()));
                _trackballEH->setCenter(volumeExtent.center());
                _trackballEH->reinitializeCamera(pos, volumeExtent.center(), _eepGenerator._camera.getValue().getUpVector());
            }
        }
        if (! _eepGenerator.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(_eepGenerator);
            lockGLContextAndExecuteProcessor(_drrraycater);
            lockGLContextAndExecuteProcessor(_simpleRaycaster);
        }
        if (! _eepGenerator.getInvalidationLevel().isValid() || !_drrraycater.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(_drrraycater);
        }
        if (! _eepGenerator.getInvalidationLevel().isValid() || !_simpleRaycaster.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(_simpleRaycaster);
        }
    }

    const std::string DVRVis::getName() const {
        return "DVRVis";
    }

}