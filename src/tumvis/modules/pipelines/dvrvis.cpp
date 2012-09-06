// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "dvrvis.h"

#include "tgt/event/keyevent.h"
#include "tgt/glcontext.h"
#include "tgt/qt/qtcontextmanager.h"
#include "core/datastructures/imagedataconverter.h"

namespace TUMVis {

    DVRVis::DVRVis()
        : VisualizationPipeline()
        , _camera("camera", "Camera")
        , _imageReader()
        , _pgGenerator()
        , _vmgGenerator()
        , _vmRenderer(_renderTargetSize)
        , _eepGenerator(_renderTargetSize)
        , _drrraycater(_renderTargetSize)
        , _simpleRaycaster(_renderTargetSize)
        , _clRaycaster(_renderTargetSize)
        , _trackballEH(0)
    {
        addProperty(&_camera);

        _trackballEH = new TrackballNavigationEventHandler(&_camera, _renderTargetSize.getValue());
        _eventHandlers.push_back(_trackballEH);

        _processors.push_back(&_imageReader);
        _processors.push_back(&_pgGenerator);
        _processors.push_back(&_vmgGenerator);
        _processors.push_back(&_vmRenderer);
        _processors.push_back(&_eepGenerator);
        _processors.push_back(&_drrraycater);
        _processors.push_back(&_simpleRaycaster);
        _processors.push_back(&_clRaycaster);
    }

    DVRVis::~DVRVis() {
        delete _trackballEH;
    }

    void DVRVis::init() {
        VisualizationPipeline::init();

        _camera.addSharedProperty(&_vmgGenerator._camera);
        _camera.addSharedProperty(&_vmRenderer._camera);
        _camera.addSharedProperty(&_eepGenerator._camera);
        _camera.addSharedProperty(&_drrraycater._camera);
        _camera.addSharedProperty(&_simpleRaycaster._camera);
        _camera.addSharedProperty(&_clRaycaster._camera);

        _imageReader._url.setValue("D:\\Medical Data\\smallHeart.mhd");
        _imageReader._targetImageID.setValue("reader.output");

        _drrraycater._targetImageID.setValue("drr.output");
        _drrraycater._sourceImageID.setValue("eep.input");

        _simpleRaycaster._targetImageID.setValue("dvr.output");
        _simpleRaycaster._sourceImageID.setValue("eep.input");

        _clRaycaster._targetImageID.setValue("clr.output");
        _clRaycaster._sourceImageID.setValue("clr.input");

        _eepGenerator._sourceImageID.setValue("eep.input");
        _pgGenerator._sourceImageID.setValue("eep.input");

        _renderTargetID.setValue("eep.entry");

        _pgGenerator._geometryID.connect(&_eepGenerator._geometryID);
        _vmgGenerator._mirrorID.connect(&_eepGenerator._mirrorID);
        _vmgGenerator._mirrorID.connect(&_vmRenderer._geometryID);

        _eepGenerator._entryImageID.connect(&_drrraycater._entryImageID);
        _eepGenerator._entryImageID.connect(&_simpleRaycaster._entryImageID);
        _eepGenerator._entryImageID.connect(&_clRaycaster._entryImageID);

        _eepGenerator._exitImageID.connect(&_drrraycater._exitImageID);
        _eepGenerator._exitImageID.connect(&_simpleRaycaster._exitImageID);
        _eepGenerator._exitImageID.connect(&_clRaycaster._exitImageID);

        _imageReader.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);
        _vmgGenerator.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);
        _vmRenderer.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);
        _pgGenerator.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);
        _eepGenerator.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);
        _drrraycater.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);
        _simpleRaycaster.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);
        _clRaycaster.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);

        _trackballEH->setViewportSize(_renderTargetSize.getValue());
        _renderTargetSize.s_changed.connect<DVRVis>(this, &DVRVis::onRenderTargetSizeChanged);
    }

    void DVRVis::execute() {
        {
            tbb::spin_mutex::scoped_lock lock(_localMutex);
            _invalidationLevel.setValid();
            // TODO:    think whether we want to lock all processors already here.
        }
        if (! _imageReader.getInvalidationLevel().isValid()) {
            executeProcessor(&_imageReader);

            // convert data
            DataContainer::ScopedTypedData<ImageData> img(_data, "reader.output");
            ImageDataLocal* local = ImageDataConverter::tryConvert<ImageDataLocal>(img);
            if (local != 0) {
                _data.addData("clr.input", local);
            }
            {
                tgt::GLContextScopedLock lock(_canvas->getContext());
                ImageDataGL* gl = ImageDataConverter::tryConvert<ImageDataGL>(img);
                if (gl != 0) {
                    _data.addData("eep.input", gl);
                }
            }
            CtxtMgr.releaseCurrentContext();

            tgt::Bounds volumeExtent = img->getWorldBounds();
            tgt::vec3 pos = volumeExtent.center() - tgt::vec3(0, 0, tgt::length(volumeExtent.diagonal()));
            
            _trackballEH->setSceneBounds(volumeExtent);
            _trackballEH->setCenter(volumeExtent.center());
            _trackballEH->reinitializeCamera(pos, volumeExtent.center(), _camera.getValue().getUpVector());

        }
        if (! _pgGenerator.getInvalidationLevel().isValid()) {
            executeProcessor(&_pgGenerator);
        }
        if (! _vmgGenerator.getInvalidationLevel().isValid()) {
            executeProcessor(&_vmgGenerator);
        }
        if (! _vmRenderer.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(&_vmRenderer);
        }
        if (! _eepGenerator.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(&_eepGenerator);
        }
        if (!_drrraycater.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(&_drrraycater);
        }
        if (!_simpleRaycaster.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(&_simpleRaycaster);
        }
        if (!_clRaycaster.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(&_clRaycaster);
        }
    }

    const std::string DVRVis::getName() const {
        return "DVRVis";
    }

    void DVRVis::onRenderTargetSizeChanged(const AbstractProperty* prop) {
        _trackballEH->setViewportSize(_renderTargetSize.getValue());
        float ratio = static_cast<float>(_renderTargetSize.getValue().x) / static_cast<float>(_renderTargetSize.getValue().y);
        _camera.setWindowRatio(ratio);
    }

}