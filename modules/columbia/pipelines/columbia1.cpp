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
#include "core/datastructures/imagedata.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    Columbia1::Columbia1(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _camera("camera", "Camera")
        , _boundsData("BoundsData", "Bounds Data", "sfr", DataNameProperty::READ)
        , _imageReader()
        , _flowReader()
        , _vtkReader()
        , _vr(&_canvasSize)
        , _sr(&_canvasSize)
        , _src(&_canvasSize)
        , _gr(&_canvasSize)
        , _sft()
        , _sfr(&_canvasSize)
        , _compositor(&_canvasSize)
        , _trackballEH(0)
    {
        addProperty(&_camera);
        addProperty(&_boundsData);

        _trackballEH = new TrackballNavigationEventListener(&_camera, &_canvasSize);
        _trackballEH->addLqModeProcessor(&_vr);
        _trackballEH->addLqModeProcessor(&_src);
        _trackballEH->addLqModeProcessor(&_sfr);
        addEventListenerToBack(_trackballEH);

        addProcessor(&_imageReader);
        addProcessor(&_imageSplitter);
        addProcessor(&_vr);

        addProcessor(&_flowReader);
        addProcessor(&_flowSplitter);

        //addProcessor(&_vtkReader);

        //addProcessor(&_src);
        addProcessor(&_sr);
        //addProcessor(&_gr);

        addProcessor(&_sft);
        addProcessor(&_sfr);

        addProcessor(&_compositor);
    }

    Columbia1::~Columbia1() {
        delete _trackballEH;
    }

    void Columbia1::init() {
        AutoEvaluationPipeline::init();
        
        _imageSplitter.s_validated.connect(this, &Columbia1::onProcessorValidated);

        _camera.addSharedProperty(&_vr.p_camera);
        _camera.addSharedProperty(&_src.p_camera);
        _camera.addSharedProperty(&_gr.p_camera);
        _camera.addSharedProperty(&_sfr.p_camera);

        _vr.p_outputImage.setValue("vr");
        _sr.p_targetImageID.setValue("sr");
        _src.p_targetImageID.setValue("src");
        _compositor.p_targetImageId.setValue("composited");

        _renderTargetID.setValue("composited");

        _imageReader.p_url.setValue("D:/Medical Data/Columbia/inputs/FullVolumeLV_3D_25Hz_[IM_0004]_NIF_diffused_crop_00.ltf");
        _imageReader.p_size.setValue(tgt::ivec3(224, 176, 208));
        _imageReader.p_numChannels.setValue(1);
        _imageReader.p_baseType.selectById("uint8");
        _imageReader.p_targetImageID.setValue("image");
        _imageReader.p_targetImageID.addSharedProperty(&_imageSplitter.p_inputID);

        _imageSplitter.p_outputID.setValue("image.single");
        _imageSplitter.p_outputID.addSharedProperty(&_vr.p_inputVolume);

        _flowReader.p_url.setValue("D:/Medical Data/Columbia/outputs/FullVolumeLV_3D_25Hz_[IM_0004]_NIF_crop_flow_field_00_00.ltf");
        _flowReader.p_size.setValue(tgt::ivec3(224, 176, 208));
        _flowReader.p_numChannels.setValue(3);
        _flowReader.p_baseType.selectById("float");
        _flowReader.p_targetImageID.setValue("flow");
        _flowReader.p_targetImageID.addSharedProperty(&_flowSplitter.p_inputID);

        _flowSplitter.p_outputID.setValue("flow.single");
        _flowSplitter.p_outputID.addSharedProperty(&_src.p_sourceImageID);
        _flowSplitter.p_outputID.addSharedProperty(&_sr.p_sourceImageID);
        _flowSplitter.p_outputID.addSharedProperty(&_sft.p_strainId);
         
        _vtkReader.p_targetImageID.setValue("mesh");
        _vtkReader.p_url.setValue("D:/Medical Data/Columbia/inputs/Myocardium Meshes/FullVolumeLV_3D_25Hz_ED_Mesh_Endo.vtk");
        _vtkReader.p_targetImageID.addSharedProperty(&_gr.p_geometryID);

        Geometry1DTransferFunction* dvrTF = new Geometry1DTransferFunction(128, tgt::vec2(0.f, 1.f));
        dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.1f, .125f), tgt::col4(255, 0, 0, 32), tgt::col4(255, 0, 0, 32)));
        dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.4f, .5f), tgt::col4(0, 255, 0, 128), tgt::col4(0, 255, 0, 128)));
        static_cast<TransferFunctionProperty*>(_vr.getProperty("TransferFunction"))->replaceTF(dvrTF);
        _vr.p_outputImage.addSharedProperty(&_compositor.p_secondImageId);

        _gr.p_renderTargetID.setValue("gr");

        _sft.p_outputID.setValue("fibers");
        _sft.p_outputID.addSharedProperty(&_sfr.p_strainId);

        _sfr.p_renderTargetID.setValue("sfr");
        _sfr.p_renderTargetID.addSharedProperty(static_cast<DataNameProperty*>(_vr.getProperty("GeometryImageId")));
        _sfr.p_renderTargetID.addSharedProperty(&_compositor.p_firstImageId);
    }

    void Columbia1::deinit() {
        _canvasSize.s_changed.disconnect(this);
        AutoEvaluationPipeline::deinit();
    }

    void Columbia1::onProcessorValidated(AbstractProcessor* processor) {
        if (processor == &_imageSplitter) {
            // update camera
            ScopedTypedData<ImageData> img(*_data, _imageSplitter.p_outputID.getValue());
            if (img != 0) {
                _trackballEH->reinitializeCamera(img);
            }
        }
    }
    
}