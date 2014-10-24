// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#include "columbia1.h"

#include "cgt/event/keyevent.h"
#include "core/datastructures/imagedata.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    Columbia1::Columbia1(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _boundsData("BoundsData", "Bounds Data", "sfr", DataNameProperty::READ)
        , _tcp(&_canvasSize)
        , _lsp()
        , _imageReader()
        , _flowReader()
        , _vtkReader()
        , _vr(&_canvasSize)
        , _src(&_canvasSize)
        , _sr(&_canvasSize)
        , _gr(&_canvasSize)
        , _sft()
        , _sfr(&_canvasSize)
        , _compositor(&_canvasSize)
    {
        addProperty(_boundsData);

        _tcp.addLqModeProcessor(&_vr);
        _tcp.addLqModeProcessor(&_src);
        _tcp.addLqModeProcessor(&_sfr);
        addEventListenerToBack(&_tcp);

        addProcessor(&_tcp);
        addProcessor(&_lsp);
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

    }

    void Columbia1::init() {
        AutoEvaluationPipeline::init();
        
        _vr.p_outputImage.setValue("vr");
        _sr.p_targetImageID.setValue("sr");
        _src.p_targetImageID.setValue("src");
        _compositor.p_targetImageId.setValue("composited");

        _renderTargetID.setValue("composited");

        _imageReader.p_url.setValue("D:/Medical Data/Columbia/inputs/FullVolumeLV_3D_25Hz_[IM_0004]_NIF_diffused_crop_00.ltf");
        _imageReader.p_size.setValue(cgt::ivec3(224, 176, 208));
        _imageReader.p_numChannels.setValue(1);
        _imageReader.p_baseType.selectById("uint8");
        _imageReader.p_targetImageID.setValue("image");
        _imageReader.p_targetImageID.addSharedProperty(&_imageSplitter.p_inputID);

        _imageSplitter.p_outputID.setValue("image.single");
        _imageSplitter.p_outputID.addSharedProperty(&_vr.p_inputVolume);
        _imageSplitter.p_outputID.addSharedProperty(&_tcp.p_image);

        _flowReader.p_url.setValue("D:/Medical Data/Columbia/outputs/FullVolumeLV_3D_25Hz_[IM_0004]_NIF_crop_flow_field_00_00.ltf");
        _flowReader.p_size.setValue(cgt::ivec3(224, 176, 208));
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

        Geometry1DTransferFunction* dvrTF = new Geometry1DTransferFunction(128, cgt::vec2(0.f, 1.f));
        dvrTF->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.1f, .125f), cgt::col4(255, 0, 0, 32), cgt::col4(255, 0, 0, 32)));
        dvrTF->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.4f, .5f), cgt::col4(0, 255, 0, 128), cgt::col4(0, 255, 0, 128)));
        static_cast<TransferFunctionProperty*>(_vr.getNestedProperty("RaycasterProps::TransferFunction"))->replaceTF(dvrTF);
        _vr.p_outputImage.addSharedProperty(&_compositor.p_secondImageId);

        _gr.p_renderTargetID.setValue("gr");

        _sft.p_outputID.setValue("fibers");
        _sft.p_outputID.addSharedProperty(&_sfr.p_strainId);

        _sfr.p_renderTargetID.setValue("sfr");
        // _sfr.p_renderTargetID.addSharedProperty(static_cast<DataNameProperty*>(_vr.getProperty("GeometryImageId")));
        _sfr.p_renderTargetID.addSharedProperty(&_compositor.p_firstImageId);
    }

    void Columbia1::deinit() {
        AutoEvaluationPipeline::deinit();
    }

}
