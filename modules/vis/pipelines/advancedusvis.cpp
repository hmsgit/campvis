// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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

#include "advancedusvis.h"

#include "tgt/event/keyevent.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    AdvancedUsVis::AdvancedUsVis()
        : VisualizationPipeline()
        , _imageReader()
        , _gvg()
        , _lhh()
        , _sliceExtractor(_effectiveRenderTargetSize)
        , _wheelHandler(&_sliceExtractor.p_sliceNumber)
        , _tfWindowingHandler(&_sliceExtractor.p_transferFunction)
    {
        addProcessor(&_imageReader);
//         addProcessor(&_gvg);
//         addProcessor(&_lhh);
        addProcessor(&_sliceExtractor);
        addEventHandler(&_wheelHandler);
        addEventHandler(&_tfWindowingHandler);
    }

    AdvancedUsVis::~AdvancedUsVis() {
    }

    void AdvancedUsVis::init() {
        VisualizationPipeline::init();

        _imageReader.p_url.setValue("D:\\Medical Data\\Dentalscan\\dental.mhd");
        _imageReader.p_targetImageID.setValue("reader.output");

        _gvg.p_inputVolume.setValue("reader.output");

//         _lhh.p_inputVolume.setValue("se.input");
//         _gvg._outputGradients.connect(&_lhh._inputGradients);

        _sliceExtractor.p_sourceImageID.setValue("reader.output");
        _sliceExtractor.p_sliceNumber.setValue(0);

        // TODO: replace this hardcoded domain by automatically determined from image min/max values
        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(128, tgt::vec2(0.f, .08f));
        tf->addGeometry(TFGeometry1D::createQuad(tgt::vec2(0.f, 1.f), tgt::col4(0, 0, 0, 0), tgt::col4(255, 255, 255, 255)));
        _sliceExtractor.p_transferFunction.replaceTF(tf);

        _renderTargetID.setValue("renderTarget");
        _renderTargetID.addSharedProperty(&(_sliceExtractor.p_targetImageID));
    }

    void AdvancedUsVis::execute() {
        {
            tbb::spin_mutex::scoped_lock lock(_localMutex);
            _invalidationLevel.setValid();
            // TODO:    think whether we want to lock all processors already here.
        }
        if (! _imageReader.getInvalidationLevel().isValid()) {
            executeProcessor(&_imageReader);

            // convert data
            ImageRepresentationLocal::ScopedRepresentation img(_data, "reader.output");
            if (img != 0) {
                _sliceExtractor.p_transferFunction.getTF()->setImageHandle(img.getDataHandle());
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

    void AdvancedUsVis::keyEvent(tgt::KeyEvent* e) {
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

    const std::string AdvancedUsVis::getName() const {
        return "AdvancedUsVis";
    }

}