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

#include "manualtissuesegmentation.h"

#include "tgt/filesystem.h"
#include "core/datastructures/imagerepresentationlocal.h"

#ifdef CAMPVIS_HAS_MODULE_DEVIL
#include <IL/il.h>
#include <IL/ilu.h>
#endif

#include <sstream>
#include <iomanip>

namespace campvis {


    ManualTissueSegmentation::ManualTissueSegmentation(DataContainer* dc) 
        : AutoEvaluationPipeline(dc)
        , _reader()
        , _mts(&_canvasSize)
        , _writer()
        , _wheelHandler(&_mts.p_frameNumber)
    {
        addProcessor(&_reader);
        addProcessor(&_mts);
        addProcessor(&_writer);

        addEventListenerToBack(&_wheelHandler);
        addEventListenerToBack(&_mts);
    }

    ManualTissueSegmentation::~ManualTissueSegmentation() {

    }

    void ManualTissueSegmentation::init() {
        AutoEvaluationPipeline::init();

        _reader.p_url.setValue(CAMPVIS_SOURCE_DIR "/modules/advancedusvis/sampledata/Volume_0_small.mhd");
        //_reader.p_url.setValue("D:/Medical Data/us_carotid_2014-02/christian/christian_bmode_2D_compounded_cropped.mhd");
        _reader.p_targetImageID.addSharedProperty(&_mts.p_sourceImageID);

        _mts.p_targetImageID.setValue("segmentation");

        _writer.p_inputImage.setValue("LabelImage");
        _writer.p_fileName.setValue(CAMPVIS_SOURCE_DIR "/modules/advancedusvis/sampledata/layers_0_small.mhd");
        //_writer.p_fileName.setValue("D:/Medical Data/us_carotid_2014-02/christian/christian_bmode_2D_compounded_cropped_layers.mhd");

        _renderTargetID.setValue(_mts.p_targetImageID.getValue());
    }

    void ManualTissueSegmentation::deinit() {
        AutoEvaluationPipeline::deinit();
    }

    void ManualTissueSegmentation::onRenderTargetSizeChanged(const AbstractProperty* prop) {
    }


}