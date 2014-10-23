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

#include "advancedusvis.h"

#include "cgt/event/keyevent.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"
#include "core/datastructures/genericimagerepresentationlocal.h"

namespace campvis {

    AdvancedUsVis::AdvancedUsVis(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _usReader()
        , _confidenceReader()
        , _usFusion1(&_canvasSize)
        , _usFusion2(&_canvasSize)
        , _usFusion3(&_canvasSize)
        , _usFusion4(&_canvasSize)
        , _usBlurFilter(&_canvasSize)
        , _quadView(&_canvasSize)
    {
        addProcessor(&_usReader);
        addProcessor(&_confidenceReader);
        addProcessor(&_usBlurFilter);
        addProcessor(&_usFusion1);
        addProcessor(&_usFusion2);
        addProcessor(&_usFusion3);
        addProcessor(&_usFusion4);
        addProcessor(&_quadView);
    }

    AdvancedUsVis::~AdvancedUsVis() {
    }

    void AdvancedUsVis::init() {
        AutoEvaluationPipeline::init();

        _usReader.p_url.setValue(CAMPVIS_SOURCE_DIR "/modules/advancedusvis/sampledata/us.mhd");
        _usReader.p_targetImageID.setValue("us.image");
        _usReader.p_targetImageID.addSharedProperty(&_usFusion1.p_usImageId);
        _usReader.p_targetImageID.addSharedProperty(&_usFusion2.p_usImageId);
        _usReader.p_targetImageID.addSharedProperty(&_usFusion3.p_usImageId);
        _usReader.p_targetImageID.addSharedProperty(&_usFusion4.p_usImageId);
        _usReader.p_targetImageID.addSharedProperty(&_usBlurFilter.p_inputImage);

        _confidenceReader.p_url.setValue(CAMPVIS_SOURCE_DIR "/modules/advancedusvis/sampledata/cm.mhd");
        _confidenceReader.p_targetImageID.setValue("confidence.image.read");
        _confidenceReader.p_targetImageID.addSharedProperty(&_usFusion1.p_confidenceImageID);
        _confidenceReader.p_targetImageID.addSharedProperty(&_usFusion2.p_confidenceImageID);
        _confidenceReader.p_targetImageID.addSharedProperty(&_usFusion3.p_confidenceImageID);
        _confidenceReader.p_targetImageID.addSharedProperty(&_usFusion4.p_confidenceImageID);

        _usFusion1.p_targetImageID.setValue("us.fused1");
        _usFusion1.p_targetImageID.addSharedProperty(&_quadView.p_inputImage1);
        _usFusion1.p_view.selectById("us");
        _usFusion1.p_sliceNumber.setValue(0);
        _usFusion1.p_sliceNumber.addSharedProperty(&_usFusion2.p_sliceNumber);
        _usFusion1.p_sliceNumber.addSharedProperty(&_usFusion3.p_sliceNumber);
        _usFusion1.p_sliceNumber.addSharedProperty(&_usFusion4.p_sliceNumber);

        _usFusion2.p_targetImageID.setValue("us.fused2");
        _usFusion2.p_targetImageID.addSharedProperty(&_quadView.p_inputImage2);
        _usFusion2.p_view.selectById("mappingSaturationHSV");

        _usFusion3.p_targetImageID.setValue("us.fused3");
        _usFusion3.p_targetImageID.addSharedProperty(&_quadView.p_inputImage3);
        _usFusion3.p_view.selectById("mappingLAB");
        _usFusion3.p_hue.setValue(0.22f);

        _usFusion4.p_targetImageID.setValue("us.fused4");
        _usFusion4.p_targetImageID.addSharedProperty(&_quadView.p_inputImage4);
        _usFusion4.p_view.selectById("mappingSharpness");

        _usBlurFilter.p_outputImage.setValue("us.blurred");
        _usBlurFilter.p_outputImage.addSharedProperty(&_usFusion1.p_blurredImageId);
        _usBlurFilter.p_outputImage.addSharedProperty(&_usFusion2.p_blurredImageId);
        _usBlurFilter.p_outputImage.addSharedProperty(&_usFusion3.p_blurredImageId);
        _usBlurFilter.p_outputImage.addSharedProperty(&_usFusion4.p_blurredImageId);
        _usBlurFilter.p_sigma.setValue(4.f);

        _quadView.p_outputImage.setValue("quadview.output");

        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(128, tgt::vec2(0.f, 1.f));
        tf->addGeometry(TFGeometry1D::createQuad(tgt::vec2(0.f, 1.f), tgt::col4(0, 0, 0, 255), tgt::col4(255, 255, 255, 255)));
        _usFusion1.p_transferFunction.replaceTF(tf);
        _usFusion2.p_transferFunction.replaceTF(tf->clone());
        _usFusion3.p_transferFunction.replaceTF(tf->clone());
        _usFusion4.p_transferFunction.replaceTF(tf->clone());

        _renderTargetID.setValue("quadview.output");
    }

    void AdvancedUsVis::deinit() {
        AutoEvaluationPipeline::deinit();
    }

    void AdvancedUsVis::keyEvent(tgt::KeyEvent* e) {
        if (e->pressed()) {
            switch (e->keyCode()) {
                case tgt::KeyEvent::K_UP:
                    _usFusion1.p_sliceNumber.increment();
                    break;
                case tgt::KeyEvent::K_DOWN:
                    _usFusion1.p_sliceNumber.decrement();
                    break;
                default:
                    break;
            }
        }
    }

}
