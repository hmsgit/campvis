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