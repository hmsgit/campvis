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

#include "reducertest.h"

#include "tgt/event/keyevent.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"
#include "core/datastructures/renderdata.h"

#include "core/tools/glreduction.h"

namespace campvis {

    ReducerTest::ReducerTest(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _referenceReader()
        , _movingReader()
        , _sm()
        , _ve(&_canvasSize)
    {
        addProcessor(&_referenceReader);
        addProcessor(&_movingReader);
        addProcessor(&_sm);
        addProcessor(&_ve);

        addEventListenerToBack(&_ve);
    }

    ReducerTest::~ReducerTest() {
    }

    void ReducerTest::init() {
        AutoEvaluationPipeline::init();

        _referenceReader.p_url.setValue("D:/Medical Data/SCR/Data/RegSweeps_phantom_cropped/-1S3median/Volume_0.mhd");
        _referenceReader.p_targetImageID.setValue("Reference Image");
        _referenceReader.p_targetImageID.addSharedProperty(&_sm.p_referenceId);

        _movingReader.p_url.setValue("D:/Medical Data/SCR/Data/RegSweeps_phantom_cropped/-1S3median/Volume_1.mhd");
        _movingReader.p_targetImageID.setValue("Moving Image");
        _movingReader.p_targetImageID.addSharedProperty(&_sm.p_movingId);

        _sm.p_differenceImageId.addSharedProperty(&_ve.p_inputVolume);

        _ve.p_outputImage.setValue("renderTarget");
        _renderTargetID.setValue("renderTarget");
    }

    void ReducerTest::onProcessorValidated(AbstractProcessor* processor) {

    }

}
