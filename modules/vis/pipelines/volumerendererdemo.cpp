// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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

#include "volumerendererdemo.h"

#include "tgt/event/keyevent.h"
#include "tgt/glcontext.h"
#include "tgt/qt/qtcontextmanager.h"
#include "core/datastructures/imagedata.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    VolumeRendererDemo::VolumeRendererDemo()
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
        addProcessor(&_vr);
    }

    VolumeRendererDemo::~VolumeRendererDemo() {
        delete _trackballEH;
    }

    void VolumeRendererDemo::init() {
        VisualizationPipeline::init();
        
        _imageReader.s_validated.connect(this, &VolumeRendererDemo::onProcessorValidated);

        _camera.addSharedProperty(&_vr.p_camera);
        _vr.p_outputImage.setValue("combine");
        _renderTargetID.setValue("combine");

        _imageReader.p_url.setValue("D:\\Medical Data\\smallHeart.mhd");
        _imageReader.p_targetImageID.setValue("reader.output");
        _imageReader.p_targetImageID.connect(&_vr.p_inputVolume);

//          Geometry1DTransferFunction* dvrTF = new Geometry1DTransferFunction(128, tgt::vec2(0.f, .05f));
//          dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.4f, .42f), tgt::col4(255, 0, 0, 255), tgt::col4(255, 0, 0, 255)));
//          dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.45f, .5f), tgt::col4(0, 255, 0, 255), tgt::col4(0, 255, 0, 255)));
//          _dvrNormal.p_transferFunction.replaceTF(dvrTF);

        _trackballEH->setViewportSize(_effectiveRenderTargetSize.getValue());
        _effectiveRenderTargetSize.s_changed.connect<VolumeRendererDemo>(this, &VolumeRendererDemo::onRenderTargetSizeChanged);
    }

    void VolumeRendererDemo::deinit() {
        _effectiveRenderTargetSize.s_changed.disconnect(this);
        VisualizationPipeline::deinit();
    }

    const std::string VolumeRendererDemo::getName() const {
        return "VolumeRendererDemo";
    }

    void VolumeRendererDemo::onRenderTargetSizeChanged(const AbstractProperty* prop) {
        _trackballEH->setViewportSize(_canvasSize.getValue());
        float ratio = static_cast<float>(_effectiveRenderTargetSize.getValue().x) / static_cast<float>(_effectiveRenderTargetSize.getValue().y);
        _camera.setWindowRatio(ratio);
    }

    void VolumeRendererDemo::onProcessorValidated(AbstractProcessor* processor) {
        if (processor == &_imageReader) {
            // update camera
            DataContainer::ScopedTypedData<ImageData> img(_data, _imageReader.p_targetImageID.getValue());
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