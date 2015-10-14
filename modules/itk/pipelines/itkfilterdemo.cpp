// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#include "itkfilterdemo.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    ItkFilterDemo::ItkFilterDemo(DataContainer& dc)
        : AutoEvaluationPipeline(dc, getId())
        , _lsp()
        , _imageReader()
        , _itkFilter()
        , _ve(&_canvasSize)
    {
        addProcessor(&_lsp);
        addProcessor(&_imageReader);
        addProcessor(&_itkFilter);
        addProcessor(&_ve);

        addEventListenerToBack(&_ve);
    }

    ItkFilterDemo::~ItkFilterDemo() {
    }

    void ItkFilterDemo::init() {
        AutoEvaluationPipeline::init();

        _ve.p_outputImage.setValue("result");
        _renderTargetID.setValue("result");

        _imageReader.p_url.setValue(CAMPVIS_SOURCE_DIR "/modules/vis/sampledata/smallHeart.mhd");
        _imageReader.p_targetImageID.setValue("reader.output");
        _imageReader.p_targetImageID.addSharedProperty(&_itkFilter.p_sourceImageID);

        _itkFilter.p_filterMode.selectById("sobel");
        _itkFilter.p_targetImageID.setValue("filtered");
        _itkFilter.p_targetImageID.addSharedProperty(&_ve.p_inputVolume);
    
        Geometry1DTransferFunction* dvrTF = new Geometry1DTransferFunction(128, cgt::vec2(0.f, .05f));
        dvrTF->addGeometry(TFGeometry1D::createQuad(cgt::vec2(.4f, .5f), cgt::col4(32, 192, 0, 128), cgt::col4(32, 192, 0, 128)));
        static_cast<TransferFunctionProperty*>(_ve.getNestedProperty("VolumeRendererProperties::RaycasterProps::TransferFunction"))->replaceTF(dvrTF);
    }

    void ItkFilterDemo::deinit() {
        _canvasSize.s_changed.disconnect(this);
        AutoEvaluationPipeline::deinit();
    }


}