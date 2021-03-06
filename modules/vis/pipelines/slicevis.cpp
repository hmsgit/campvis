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

#include "slicevis.h"

#include "cgt/event/keyevent.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    SliceVis::SliceVis(DataContainer& dc)
        : AutoEvaluationPipeline(dc, getId())
        , _imageReader()
        , _sliceExtractor(&_canvasSize)
        , _wheelHandler(&_sliceExtractor.p_zSliceNumber)
        , _tfWindowingHandler(&_sliceExtractor.p_transferFunction)
    {
        addProcessor(&_imageReader);
        addProcessor(&_sliceExtractor);
        addEventListenerToBack(&_wheelHandler);
        addEventListenerToBack(&_tfWindowingHandler);
    }

    SliceVis::~SliceVis() {
    }

    void SliceVis::init() {
        AutoEvaluationPipeline::init();

        _imageReader.p_url.setValue(ShdrMgr.completePath("/modules/vis/sampledata/smallHeart.mhd"));
        _imageReader.p_targetImageID.setValue("reader.output");
        _imageReader.p_targetImageID.addSharedProperty(&_sliceExtractor.p_sourceImageID);
        _imageReader.s_validated.connect(this, &SliceVis::onProcessorValidated);

        _sliceExtractor.p_xSliceNumber.setValue(0);

        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(128, cgt::vec2(0.f, .08f));
        tf->addGeometry(TFGeometry1D::createQuad(cgt::vec2(0.f, 1.f), cgt::col4(0, 0, 0, 0), cgt::col4(255, 255, 255, 255)));
        _sliceExtractor.p_transferFunction.replaceTF(tf);

        _renderTargetID.setValue("renderTarget");
        _renderTargetID.addSharedProperty(&(_sliceExtractor.p_targetImageID));
    }

    void SliceVis::keyEvent(cgt::KeyEvent* e) {
        if (e->pressed()) {
            switch (e->keyCode()) {
                case cgt::KeyEvent::K_UP:
                    _sliceExtractor.p_xSliceNumber.increment();
                    break;
                case cgt::KeyEvent::K_DOWN:
                    _sliceExtractor.p_xSliceNumber.decrement();
                    break;
                default:
                    break;
            }
        }
    }

    void SliceVis::onProcessorValidated(AbstractProcessor* processor) {
        if (processor == &_imageReader) {
            ScopedTypedData<ImageData> img(*_dataContainer, _imageReader.p_targetImageID.getValue());
            if (img != 0) {
                _sliceExtractor.p_transferFunction.setImageHandle(img.getDataHandle());
            }
        }
    }

}
