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

#include "openclpipeline.h"

#include "tgt/event/keyevent.h"
#include "tgt/glcontext.h"
#include "tgt/qt/qtcontextmanager.h"
#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    OpenCLPipeline::OpenCLPipeline()
        : VisualizationPipeline()
        , _camera("camera", "Camera")
        , _imageReader()
        , _pgGenerator()
        , _eepGenerator(_effectiveRenderTargetSize)
        , _clRaycaster(_effectiveRenderTargetSize)
        , _trackballEH(0)
    {
        addProperty(&_camera);

        _trackballEH = new TrackballNavigationEventHandler(this, &_camera, _renderTargetSize);
        _eventHandlers.push_back(_trackballEH);

        addProcessor(&_imageReader);
        addProcessor(&_pgGenerator);
        addProcessor(&_eepGenerator);
        addProcessor(&_clRaycaster);
    }

    OpenCLPipeline::~OpenCLPipeline() {
        delete _trackballEH;
    }

    void OpenCLPipeline::init() {
        VisualizationPipeline::init();

        _camera.addSharedProperty(&_eepGenerator.p_camera);
        _camera.addSharedProperty(&_clRaycaster._camera);

        //_imageReader.p_url.setValue("D:\\Medical Data\\Dentalscan\\dental.mhd");
        _imageReader.p_url.setValue("D:\\Medical Data\\smallHeart.mhd");
        _imageReader.p_targetImageID.setValue("reader.output");
        _imageReader.p_targetImageID.connect(&_pgGenerator.p_sourceImageID);
        _imageReader.p_targetImageID.connect(&_eepGenerator.p_sourceImageID);

        _clRaycaster._targetImageID.setValue("cl.output");
        _clRaycaster._sourceImageID.setValue("clr.input");

         Geometry1DTransferFunction* dvrTF = new Geometry1DTransferFunction(128, tgt::vec2(0.f, .05f));
         dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.4f, .42f), tgt::col4(255, 0, 0, 255), tgt::col4(255, 0, 0, 255)));
         dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.45f, .5f), tgt::col4(0, 255, 0, 255), tgt::col4(0, 255, 0, 255)));
         _clRaycaster._transferFunction.replaceTF(dvrTF);



        _renderTargetID.setValue("cl.output");

        _pgGenerator.p_geometryID.connect(&_eepGenerator.p_geometryID);

        _eepGenerator.p_entryImageID.connect(&_clRaycaster._entryImageID);
        _eepGenerator.p_exitImageID.connect(&_clRaycaster._exitImageID);

        _trackballEH->setViewportSize(_effectiveRenderTargetSize.getValue());
        _effectiveRenderTargetSize.s_changed.connect<OpenCLPipeline>(this, &OpenCLPipeline::onRenderTargetSizeChanged);
    }

    void OpenCLPipeline::execute() {
        {
            tbb::spin_mutex::scoped_lock lock(_localMutex);
            _invalidationLevel.setValid();
            // TODO:    think whether we want to lock all processors already here.
        }
        if (! _imageReader.getInvalidationLevel().isValid()) {
            executeProcessor(&_imageReader);

            // convert data
            ImageRepresentationLocal::ScopedRepresentation img(_data, "reader.output");
            if (img != 0) {
                size_t numElements = img->getNumElements();
                float* asFloats = new float[numElements];
                for (size_t i = 0; i < numElements; ++i)
                    asFloats[i] = img->getElementNormalized(i, 0);
                ImageData* id = new ImageData(img->getDimensionality(), img->getSize(), img->getParent()->getNumChannels());
                GenericImageRepresentationLocal<float, 1>* imageWithFloats = GenericImageRepresentationLocal<float, 1>::create(id, asFloats);

                DataHandle dh = _data.addData("clr.input", id);

                tgt::Bounds volumeExtent = img->getParent()->getWorldBounds();
                tgt::vec3 pos = volumeExtent.center() - tgt::vec3(0, 0, tgt::length(volumeExtent.diagonal()));

                _trackballEH->setSceneBounds(volumeExtent);
                _trackballEH->setCenter(volumeExtent.center());
                _trackballEH->reinitializeCamera(pos, volumeExtent.center(), _camera.getValue().getUpVector());
            }

        }
        if (! _pgGenerator.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(&_pgGenerator);
        }
        if (! _eepGenerator.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(&_eepGenerator);
        }
        if (! _clRaycaster.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(&_clRaycaster);
        }
    }

    const std::string OpenCLPipeline::getName() const {
        return "OpenCLPipeline";
    }

    void OpenCLPipeline::onRenderTargetSizeChanged(const AbstractProperty* prop) {
        _trackballEH->setViewportSize(_renderTargetSize);
        float ratio = static_cast<float>(_effectiveRenderTargetSize.getValue().x) / static_cast<float>(_effectiveRenderTargetSize.getValue().y);
        _camera.setWindowRatio(ratio);
    }


}