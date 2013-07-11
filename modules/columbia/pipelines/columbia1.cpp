// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
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

#include "columbia1.h"

#include "tgt/event/keyevent.h"
#include "tgt/glcontext.h"
#include "tgt/qt/qtcontextmanager.h"
#include "core/datastructures/imagedata.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    Columbia1::Columbia1()
        : VisualizationPipeline()
        , _camera("camera", "Camera")
        , _imageReader()
        , _vr(_effectiveRenderTargetSize)
        , _trackballEH(0)
    {
        addProperty(&_camera);

        _trackballEH = new TrackballNavigationEventHandler(this, &_camera, _canvasSize.getValue());
        _eventHandlers.push_back(_trackballEH);

        addProcessor(&_imageReader);
        addProcessor(&_splitter);
        addProcessor(&_vr);
    }

    Columbia1::~Columbia1() {
        delete _trackballEH;
    }

    void Columbia1::init() {
        VisualizationPipeline::init();
        
        _splitter.s_validated.connect(this, &Columbia1::onProcessorValidated);

        _camera.addSharedProperty(&_vr.p_camera);
        _vr.p_outputImage.setValue("vr");
        _renderTargetID.setValue("vr");

        _imageReader.p_url.setValue("D:/Medical Data/Columbia/inputs/FullVolumeLV_3D_25Hz_[IM_0004]_NIF_diffused_crop_00.ltf");
        _imageReader.p_size.setValue(tgt::ivec3(224, 176, 208));
        _imageReader.p_targetImageID.setValue("reader.output");
        _imageReader.p_targetImageID.connect(&_splitter.p_inputID);

        _splitter.p_outputID.connect(&_vr.p_inputVolume);

        Geometry1DTransferFunction* dvrTF = new Geometry1DTransferFunction(128, tgt::vec2(0.f, 1.f));
        dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.1f, .125f), tgt::col4(255, 0, 0, 32), tgt::col4(255, 0, 0, 32)));
        dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.4f, .5f), tgt::col4(0, 255, 0, 128), tgt::col4(0, 255, 0, 128)));
        static_cast<TransferFunctionProperty*>(_vr.getProperty("transferFunction"))->replaceTF(dvrTF);

        _trackballEH->setViewportSize(_effectiveRenderTargetSize.getValue());
        _effectiveRenderTargetSize.s_changed.connect<Columbia1>(this, &Columbia1::onRenderTargetSizeChanged);
    }

    void Columbia1::deinit() {
        _effectiveRenderTargetSize.s_changed.disconnect(this);
        VisualizationPipeline::deinit();
    }

    const std::string Columbia1::getName() const {
        return "Columbia1";
    }

    void Columbia1::onRenderTargetSizeChanged(const AbstractProperty* prop) {
        _trackballEH->setViewportSize(_canvasSize.getValue());
        float ratio = static_cast<float>(_effectiveRenderTargetSize.getValue().x) / static_cast<float>(_effectiveRenderTargetSize.getValue().y);
        _camera.setWindowRatio(ratio);
    }

    void Columbia1::onProcessorValidated(AbstractProcessor* processor) {
        if (processor == &_splitter) {
            // update camera
            DataContainer::ScopedTypedData<ImageData> img(_data, _splitter.p_outputID.getValue());
            if (img != 0) {
                tgt::Bounds volumeExtent = img->getWorldBounds();
                tgt::vec3 pos = volumeExtent.center() - tgt::vec3(0, 0, tgt::length(volumeExtent.diagonal()));

                _trackballEH->setSceneBounds(volumeExtent);
                _trackballEH->setCenter(volumeExtent.center());
                _trackballEH->reinitializeCamera(pos, volumeExtent.center(), _camera.getValue().getUpVector());
            }
        }
    }


}