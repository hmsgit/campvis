// ================================================================================================
// 
// This file is part of the CAMPVis Visualization Framework.
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
#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    DVRVis::DVRVis()
        : VisualizationPipeline()
        , _camera("camera", "Camera")
        , _imageReader()
        , _pgGenerator()
        , _vmgGenerator()
        , _vmRenderer(_renderTargetSize)
        , _eepGenerator(_renderTargetSize)
        , _vmEepGenerator(_renderTargetSize)
        , _dvrNormal(_renderTargetSize)
        , _dvrVM(_renderTargetSize)
        , _depthDarkening(_renderTargetSize)
        , _combine(_renderTargetSize)
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
        _processors.push_back(&_vmEepGenerator);
        _processors.push_back(&_dvrNormal);
        _processors.push_back(&_dvrVM);
        _processors.push_back(&_depthDarkening);
        _processors.push_back(&_combine);
    }

    DVRVis::~DVRVis() {
        delete _trackballEH;
    }

    void DVRVis::init() {
        VisualizationPipeline::init();

        _camera.addSharedProperty(&_vmgGenerator._camera);
        _camera.addSharedProperty(&_vmRenderer._camera);
        _camera.addSharedProperty(&_eepGenerator._camera);
        _camera.addSharedProperty(&_vmEepGenerator._camera);
        _camera.addSharedProperty(&_dvrNormal._camera);
        _camera.addSharedProperty(&_dvrVM._camera);

        //_imageReader._url.setValue("D:\\Medical Data\\Dentalscan\\dental.mhd");
        _imageReader._url.setValue("D:\\Medical Data\\smallHeart.mhd");
        _imageReader._targetImageID.setValue("reader.output");

        _dvrNormal._targetImageID.setValue("drr.output");
        _dvrNormal._sourceImageID.setValue("eep.input");

         Geometry1DTransferFunction* dvrTF = new Geometry1DTransferFunction(128, tgt::vec2(0.f, .05f));
         dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.4f, .42f), tgt::col4(255, 0, 0, 255), tgt::col4(255, 0, 0, 255)));
         dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.45f, .5f), tgt::col4(0, 255, 0, 255), tgt::col4(0, 255, 0, 255)));
         _dvrNormal._transferFunction.replaceTF(dvrTF);

         Geometry1DTransferFunction* vmTF = new Geometry1DTransferFunction(128, tgt::vec2(0.f, .05f));
         vmTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.4f, .42f), tgt::col4(255, 0, 0, 255), tgt::col4(255, 0, 0, 255)));
         vmTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.45f, .5f), tgt::col4(0, 255, 0, 255), tgt::col4(0, 255, 0, 255)));
         _dvrVM._transferFunction.replaceTF(vmTF);

        _dvrVM._targetImageID.setValue("dvr.output");
        _dvrVM._sourceImageID.setValue("eep.input");

        _eepGenerator._sourceImageID.setValue("eep.input");
        _vmEepGenerator._sourceImageID.setValue("eep.input");
        _pgGenerator._sourceImageID.setValue("eep.input");

        _vmRenderer._renderTargetID.connect(&_combine._mirrorRenderID);
        _vmEepGenerator._entryImageID.setValue("vm.eep.entry");
        _vmEepGenerator._exitImageID.setValue("vm.eep.exit");
        _vmEepGenerator._enableMirror.setValue(true);

        // not the most beautiful way... *g*
        // this will all get better with scripting support.
        static_cast<BoolProperty*>(_vmEepGenerator.getProperty("applyMask"))->setValue(true);
        _vmRenderer._renderTargetID.connect(static_cast<DataNameProperty*>(_vmEepGenerator.getProperty("maskID")));

        _renderTargetID.setValue("combine");

        _pgGenerator._geometryID.connect(&_vmEepGenerator._geometryID);
        _vmgGenerator._mirrorID.connect(&_vmEepGenerator._mirrorID);
        _vmgGenerator._mirrorID.connect(&_vmRenderer._geometryID);
        _vmgGenerator._mirrorCenter.setValue(tgt::vec3(0.f, 0.f, -20.f));
        _vmgGenerator._poi.setValue(tgt::vec3(40.f, 40.f, 40.f));
        _vmgGenerator._size.setValue(60.f);

        _eepGenerator._entryImageID.connect(&_dvrNormal._entryImageID);
        _vmEepGenerator._entryImageID.connect(&_dvrVM._entryImageID);

        _eepGenerator._exitImageID.connect(&_dvrNormal._exitImageID);
        _vmEepGenerator._exitImageID.connect(&_dvrVM._exitImageID);

        _dvrVM._targetImageID.connect(&_combine._mirrorImageID);
        _combine._targetImageID.setValue("combine");

        _dvrNormal._targetImageID.connect(&_depthDarkening._inputImage);
        _depthDarkening._outputImage.connect(&_combine._normalImageID);

        _imageReader.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);
        _vmgGenerator.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);
        _vmRenderer.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);
        _pgGenerator.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);
        _eepGenerator.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);
        _vmEepGenerator.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);
        _dvrNormal.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);
        _dvrVM.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);
        _depthDarkening.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);
        _combine.s_invalidated.connect<DVRVis>(this, &DVRVis::onProcessorInvalidated);

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
            if (img != 0) {
                ImageDataLocal* local = ImageDataConverter::tryConvert<ImageDataLocal>(img);
                if (local != 0) {
                    DataHandle dh = _data.addData("clr.input", local);
                    _dvrNormal._transferFunction.getTF()->setImageHandle(dh);
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

        }
        if (! _pgGenerator.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(&_pgGenerator);
        }
        if (! _vmgGenerator.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(&_vmgGenerator);
        }
        if (! _vmRenderer.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(&_vmRenderer);
        }
        if (! _eepGenerator.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(&_eepGenerator);
        }
        if (! _vmEepGenerator.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(&_vmEepGenerator);
        }
        if (!_dvrNormal.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(&_dvrNormal);
            lockGLContextAndExecuteProcessor(&_depthDarkening);
        }
        if (!_dvrVM.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(&_dvrVM);
            lockGLContextAndExecuteProcessor(&_combine);
        }
        if (!_depthDarkening.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(&_depthDarkening);
        }
        if (!_combine.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(&_combine);
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