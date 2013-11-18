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

#include "slicevis.h"

#include "tgt/event/keyevent.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    SliceVis::SliceVis(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
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

        _imageReader.p_url.setValue("D:\\Medical Data\\Dentalscan\\dental.mhd");
        _imageReader.p_targetImageID.setValue("reader.output");
        _imageReader.p_targetImageID.addSharedProperty(&_sliceExtractor.p_sourceImageID);
        _imageReader.s_validated.connect(this, &SliceVis::onProcessorValidated);

        _sliceExtractor.p_xSliceNumber.setValue(0);

        // TODO: replace this hardcoded domain by automatically determined from image min/max values
        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(128, tgt::vec2(0.f, .08f));
        tf->addGeometry(TFGeometry1D::createQuad(tgt::vec2(0.f, 1.f), tgt::col4(0, 0, 0, 0), tgt::col4(255, 255, 255, 255)));
        _sliceExtractor.p_transferFunction.replaceTF(tf);

        _renderTargetID.setValue("renderTarget");
        _renderTargetID.addSharedProperty(&(_sliceExtractor.p_targetImageID));
    }

    void SliceVis::keyEvent(tgt::KeyEvent* e) {
        if (e->pressed()) {
            switch (e->keyCode()) {
                case tgt::KeyEvent::K_UP:
                    _sliceExtractor.p_xSliceNumber.increment();
                    break;
                case tgt::KeyEvent::K_DOWN:
                    _sliceExtractor.p_xSliceNumber.decrement();
                    break;
                default:
                    break;
            }
        }
    }

    void SliceVis::onProcessorValidated(AbstractProcessor* processor) {
        if (processor == &_imageReader) {
            ScopedTypedData<ImageData> img(*_data, _imageReader.p_targetImageID.getValue());
            if (img != 0) {
                _sliceExtractor.p_transferFunction.getTF()->setImageHandle(img.getDataHandle());
            }
        }
    }

}
