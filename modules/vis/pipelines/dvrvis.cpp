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

#include "dvrvis.h"

#include "tgt/event/keyevent.h"
#include "tgt/glcontext.h"
#include "tgt/qt/qtcontextmanager.h"
#include "core/datastructures/imagedata.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    DVRVis::DVRVis()
        : AutoEvaluationPipeline()
        , _camera("camera", "Camera")
        , _imageReader()
        , _pgGenerator()
        , _vmgGenerator()
        , _vmRenderer(&_canvasSize)
        , _eepGenerator(&_canvasSize)
        , _vmEepGenerator(&_canvasSize)
        , _dvrNormal(&_canvasSize)
        , _dvrVM(&_canvasSize)
        , _depthDarkening(&_canvasSize)
        , _combine(&_canvasSize)
        , _trackballEH(0)
    {
        addProperty(&_camera);

        _trackballEH = new TrackballNavigationEventListener(&_camera, &_canvasSize);
        _trackballEH->addLqModeProcessor(&_dvrNormal);
        _trackballEH->addLqModeProcessor(&_dvrVM);
        _trackballEH->addLqModeProcessor(&_depthDarkening);
        addEventListenerToBack(_trackballEH);

        addProcessor(&_imageReader);
        addProcessor(&_pgGenerator);
        addProcessor(&_vmgGenerator);
        addProcessor(&_vmRenderer);
        addProcessor(&_eepGenerator);
        addProcessor(&_vmEepGenerator);
        addProcessor(&_dvrNormal);
        addProcessor(&_dvrVM);
        addProcessor(&_depthDarkening);
        addProcessor(&_combine);
    }

    DVRVis::~DVRVis() {
        delete _trackballEH;
    }

    void DVRVis::init() {
        AutoEvaluationPipeline::init();
        
        _imageReader.s_validated.connect(this, &DVRVis::onProcessorValidated);

        _camera.addSharedProperty(&_vmgGenerator.p_camera);
        _camera.addSharedProperty(&_vmRenderer.p_camera);
        _camera.addSharedProperty(&_eepGenerator.p_camera);
        _camera.addSharedProperty(&_vmEepGenerator.p_camera);
        _camera.addSharedProperty(&_dvrNormal.p_camera);
        _camera.addSharedProperty(&_dvrVM.p_camera);

        //_imageReader.p_url.setValue("D:\\Medical Data\\Dentalscan\\dental.mhd");
        _imageReader.p_url.setValue("D:\\Medical Data\\smallHeart.mhd");
        _imageReader.p_targetImageID.setValue("reader.output");
        _imageReader.p_targetImageID.connect(&_eepGenerator.p_sourceImageID);
        _imageReader.p_targetImageID.connect(&_vmEepGenerator.p_sourceImageID);
        _imageReader.p_targetImageID.connect(&_dvrVM.p_sourceImageID);
        _imageReader.p_targetImageID.connect(&_dvrNormal.p_sourceImageID);
        _imageReader.p_targetImageID.connect(&_pgGenerator.p_sourceImageID);

        _dvrNormal.p_targetImageID.setValue("drr.output");
        _dvrVM.p_targetImageID.setValue("dvr.output");

         Geometry1DTransferFunction* dvrTF = new Geometry1DTransferFunction(128, tgt::vec2(0.f, .05f));
         dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.4f, .42f), tgt::col4(255, 0, 0, 255), tgt::col4(255, 0, 0, 255)));
         dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.45f, .5f), tgt::col4(0, 255, 0, 255), tgt::col4(0, 255, 0, 255)));
         _dvrNormal.p_transferFunction.replaceTF(dvrTF);

         Geometry1DTransferFunction* vmTF = new Geometry1DTransferFunction(128, tgt::vec2(0.f, .05f));
         vmTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.4f, .42f), tgt::col4(255, 0, 0, 255), tgt::col4(255, 0, 0, 255)));
         vmTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.45f, .5f), tgt::col4(0, 255, 0, 255), tgt::col4(0, 255, 0, 255)));
         _dvrVM.p_transferFunction.replaceTF(vmTF);

        _vmRenderer.p_renderTargetID.connect(&_combine.p_mirrorRenderID);
        _vmEepGenerator.p_entryImageID.setValue("vm.eep.entry");
        _vmEepGenerator.p_exitImageID.setValue("vm.eep.exit");
        _vmEepGenerator.p_enableMirror.setValue(true);

        // not the most beautiful way... *g*
        // this will all get better with scripting support.
        static_cast<BoolProperty*>(_vmEepGenerator.getProperty("applyMask"))->setValue(true);
        _vmRenderer.p_renderTargetID.connect(static_cast<DataNameProperty*>(_vmEepGenerator.getProperty("maskID")));

        _renderTargetID.setValue("combine");

        _pgGenerator.p_geometryID.connect(&_vmEepGenerator.p_geometryID);
        _pgGenerator.p_geometryID.connect(&_eepGenerator.p_geometryID);
        _vmgGenerator.p_mirrorID.connect(&_vmEepGenerator.p_mirrorID);
        _vmgGenerator.p_mirrorID.connect(&_vmRenderer.p_geometryID);
        _vmgGenerator.p_mirrorCenter.setValue(tgt::vec3(0.f, 0.f, -20.f));
        _vmgGenerator.p_poi.setValue(tgt::vec3(40.f, 40.f, 40.f));
        _vmgGenerator.p_size.setValue(60.f);

        _eepGenerator.p_entryImageID.connect(&_dvrNormal.p_entryImageID);
        _vmEepGenerator.p_entryImageID.connect(&_dvrVM.p_entryImageID);

        _eepGenerator.p_exitImageID.connect(&_dvrNormal.p_exitImageID);
        _vmEepGenerator.p_exitImageID.connect(&_dvrVM.p_exitImageID);

        _dvrVM.p_targetImageID.connect(&_combine.p_mirrorImageID);
        _combine.p_targetImageID.setValue("combine");

        _dvrNormal.p_targetImageID.connect(&_depthDarkening.p_inputImage);
        _depthDarkening.p_outputImage.connect(&_combine.p_normalImageID);
    }

    void DVRVis::deinit() {
        _canvasSize.s_changed.disconnect(this);
        AutoEvaluationPipeline::deinit();
    }

    const std::string DVRVis::getName() const {
        return "DVRVis";
    }

    void DVRVis::onProcessorValidated(AbstractProcessor* processor) {
        if (processor == &_imageReader) {
            // update camera
            DataContainer::ScopedTypedData<ImageData> img(_data, _imageReader.p_targetImageID.getValue());
            if (img != 0) {
                _trackballEH->reinitializeCamera(img);
            }
        }
    }


}