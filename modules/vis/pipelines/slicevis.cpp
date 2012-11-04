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
#include "core/datastructures/imagedataconverter.h"

namespace campvis {

    SliceVis::SliceVis()
        : VisualizationPipeline()
        , _imageReader()
        , _gvg()
        , _lhh()
        , _sliceExtractor(_renderTargetSize)
        , _wheelHandler(&_sliceExtractor.p_sliceNumber)
    {
        addProcessor(&_imageReader);
//         addProcessor(&_gvg);
//         addProcessor(&_lhh);
        addProcessor(&_sliceExtractor);
        addEventHandler(&_wheelHandler);
    }

    SliceVis::~SliceVis() {
    }

    void SliceVis::init() {
        VisualizationPipeline::init();

        _imageReader.p_url.setValue("D:\\Medical Data\\Dentalscan\\dental.mhd");
        _imageReader.p_targetImageID.setValue("reader.output");

        _gvg.p_inputVolume.setValue("se.input");

//         _lhh.p_inputVolume.setValue("se.input");
//         _gvg._outputGradients.connect(&_lhh._inputGradients);

        _sliceExtractor.p_sourceImageID.setValue("se.input");
        _sliceExtractor.p_sliceNumber.setValue(0);
        // TODO: replace this hardcoded domain by automatically determined from image min/max values
        _sliceExtractor.p_transferFunction.getTF()->setIntensityDomain(tgt::vec2(0, 0.05f));

        _renderTargetID.setValue("renderTarget");
        _renderTargetID.addSharedProperty(&(_sliceExtractor.p_targetImageID));

        _imageReader.s_invalidated.connect<SliceVis>(this, &SliceVis::onProcessorInvalidated);
        _gvg.s_invalidated.connect<SliceVis>(this, &SliceVis::onProcessorInvalidated);
        _sliceExtractor.s_invalidated.connect<SliceVis>(this, &SliceVis::onProcessorInvalidated);
    }

    void SliceVis::execute() {
        {
            tbb::spin_mutex::scoped_lock lock(_localMutex);
            _invalidationLevel.setValid();
            // TODO:    think whether we want to lock all processors already here.
        }
        if (! _imageReader.getInvalidationLevel().isValid()) {
            executeProcessor(&_imageReader);

            // convert data
            DataContainer::ScopedTypedData<ImageData> img(_data, "reader.output");
            ImageDataLocal* local = ImageDataConverter::tryConvert<ImageDataLocal>(img);
            if (local != 0) {
                _data.addData("se.input", local);
            }
        }
//         if (! _gvg.getInvalidationLevel().isValid()) {
//             executeProcessor(&_gvg);
//         }
//         if (! _lhh.getInvalidationLevel().isValid()) {
//             lockGLContextAndExecuteProcessor(&_lhh);
//         }
        if (! _sliceExtractor.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(&_sliceExtractor);
        }
    }

    void SliceVis::keyEvent(tgt::KeyEvent* e) {
        if (e->pressed()) {
            switch (e->keyCode()) {
                case tgt::KeyEvent::K_UP:
                    _sliceExtractor.p_sliceNumber.increment();
                    break;
                case tgt::KeyEvent::K_DOWN:
                    _sliceExtractor.p_sliceNumber.decrement();
                    break;
            }
        }
    }

    const std::string SliceVis::getName() const {
        return "SliceVis";
    }

}