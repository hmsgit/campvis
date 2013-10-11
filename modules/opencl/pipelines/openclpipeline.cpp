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
#include "core/datastructures/imagedata.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    OpenCLPipeline::OpenCLPipeline(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _camera("camera", "Camera")
        , _imageReader()
        , _pgGenerator()
        , _eepGenerator(&_canvasSize)
        , _clRaycaster(&_canvasSize)
        , _trackballEH(0)
    {
        addProperty(&_camera);

        _trackballEH = new TrackballNavigationEventListener(&_camera, &_canvasSize);
        _trackballEH->addLqModeProcessor(&_clRaycaster);
        addEventListenerToBack(_trackballEH);

        addProcessor(&_imageReader);
        addProcessor(&_pgGenerator);
        addProcessor(&_eepGenerator);
        addProcessor(&_clRaycaster);
    }

    OpenCLPipeline::~OpenCLPipeline() {
        delete _trackballEH;
    }

    void OpenCLPipeline::init() {
        AutoEvaluationPipeline::init();

        _camera.addSharedProperty(&_eepGenerator.p_camera);
        _camera.addSharedProperty(&_clRaycaster._camera);

        //_imageReader.p_url.setValue("D:\\Medical Data\\Dentalscan\\dental.mhd");
        _imageReader.p_url.setValue("D:\\Medical Data\\smallHeart.mhd");
        _imageReader.p_targetImageID.setValue("reader.output");
        _imageReader.p_targetImageID.addSharedProperty(&_pgGenerator.p_sourceImageID);
        _imageReader.p_targetImageID.addSharedProperty(&_eepGenerator.p_sourceImageID);
        _imageReader.s_validated.connect(this, &OpenCLPipeline::onProcessorValidated);

        _clRaycaster._targetImageID.setValue("cl.output");
        _clRaycaster._sourceImageID.setValue("clr.input");

         Geometry1DTransferFunction* dvrTF = new Geometry1DTransferFunction(128, tgt::vec2(0.f, .05f));
         dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.4f, .42f), tgt::col4(255, 0, 0, 255), tgt::col4(255, 0, 0, 255)));
         dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.45f, .5f), tgt::col4(0, 255, 0, 255), tgt::col4(0, 255, 0, 255)));
         _clRaycaster._transferFunction.replaceTF(dvrTF);



        _renderTargetID.setValue("cl.output");

        _pgGenerator.p_geometryID.addSharedProperty(&_eepGenerator.p_geometryID);

        _eepGenerator.p_entryImageID.addSharedProperty(&_clRaycaster._entryImageID);
        _eepGenerator.p_exitImageID.addSharedProperty(&_clRaycaster._exitImageID);
    }

    void OpenCLPipeline::deinit() {
        _canvasSize.s_changed.disconnect(this);
        AutoEvaluationPipeline::deinit();
    }

    void OpenCLPipeline::onProcessorValidated(AbstractProcessor* processor) {
        if (processor == &_imageReader) {
            // update camera
            ImageRepresentationLocal::ScopedRepresentation img(*_data, "reader.output");
            if (img != 0) {
                size_t numElements = img->getNumElements();
                float* asFloats = new float[numElements];
                for (size_t i = 0; i < numElements; ++i)
                    asFloats[i] = img->getElementNormalized(i, 0);
                ImageData* id = new ImageData(img->getDimensionality(), img->getSize(), img->getParent()->getNumChannels());
                GenericImageRepresentationLocal<float, 1>* imageWithFloats = GenericImageRepresentationLocal<float, 1>::create(id, asFloats);
                DataHandle dh = _data->addData("clr.input", id);
                _trackballEH->reinitializeCamera(img->getParent());
            }
        }
    }



}