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

#include "advancedusvis.h"

#include "tgt/event/keyevent.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    AdvancedUsVis::AdvancedUsVis()
        : VisualizationPipeline()
        , _usReader()
        , _confidenceReader()
        , _gvg()
        , _lhh()
        , _usFusion(_effectiveRenderTargetSize)
        , _usFilter()
        , _wheelHandler(&_usFusion.p_sliceNumber)
        , _tfWindowingHandler(&_usFusion.p_transferFunction)
    {
        addProcessor(&_usReader);
        addProcessor(&_confidenceReader);
        addProcessor(&_gvg);
        //addProcessor(&_lhh);
        addProcessor(&_usFusion);
        addProcessor(&_usFilter);
        addEventHandler(&_wheelHandler);
        addEventHandler(&_tfWindowingHandler);
    }

    AdvancedUsVis::~AdvancedUsVis() {
    }

    void AdvancedUsVis::init() {
        VisualizationPipeline::init();

        _usReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\01\\BMode_01.mhd");
        _usReader.p_targetImageID.setValue("us.image");
        _usReader.p_targetImageID.connect(&_usFusion.p_usImageId);
        _usReader.p_targetImageID.connect(&_gvg.p_sourceImageID);
        _usReader.p_targetImageID.connect(&_lhh.p_intensitiesId);
        _usReader.p_targetImageID.connect(&_usFilter.p_sourceImageID);

        _confidenceReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\01\\Confidence_01.mhd");
        _confidenceReader.p_targetImageID.setValue("confidence.image");
        _confidenceReader.p_targetImageID.connect(&_usFusion.p_confidenceImageID);

        _gvg.p_targetImageID.connect(&_lhh.p_gradientsId);
        _gvg.p_targetImageID.connect(&_usFusion.p_gradientImageID);

        _usFusion.p_targetImageID.setValue("us.fused");
        _usFusion.p_sliceNumber.setValue(0);
        _usFusion.p_view.setValue(0);

        _usFilter.p_targetImageID.setValue("us.filtered");
        _usFilter.p_targetImageID.connect(&_usFusion.p_blurredImageId);
        _usFilter.p_filterMode.selectById("gauss");
        _usFilter.p_sigma.setValue(4.f);

        // TODO: replace this hardcoded domain by automatically determined from image min/max values
        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(128, tgt::vec2(0.f, 1.f));
        tf->addGeometry(TFGeometry1D::createQuad(tgt::vec2(0.f, 1.f), tgt::col4(0, 0, 0, 0), tgt::col4(255, 255, 255, 255)));
        _usFusion.p_transferFunction.replaceTF(tf);

        _usFusion.p_targetImageID.addSharedProperty(&(_renderTargetID));
    }

    void AdvancedUsVis::execute() {
        {
            tbb::spin_mutex::scoped_lock lock(_localMutex);
            _invalidationLevel.setValid();
            // TODO:    think whether we want to lock all processors already here.
        }

        for (std::vector<AbstractProcessor*>::iterator it = _processors.begin(); it != _processors.end(); ++it) {
            if (! (*it)->getInvalidationLevel().isValid())
                lockGLContextAndExecuteProcessor(*it);
        }
//         if (! _usReader.getInvalidationLevel().isValid()) {
//             executeProcessor(&_usReader);
//         }
//         if (! _gvg.getInvalidationLevel().isValid()) {
//             executeProcessor(&_gvg);
//         }
//         if (! _lhh.getInvalidationLevel().isValid()) {
//             lockGLContextAndExecuteProcessor(&_lhh);
//         }
//         if (! _usFusion.getInvalidationLevel().isValid()) {
//             lockGLContextAndExecuteProcessor(&_usFusion);
//         }
    }

    void AdvancedUsVis::keyEvent(tgt::KeyEvent* e) {
        if (e->pressed()) {
            switch (e->keyCode()) {
                case tgt::KeyEvent::K_UP:
                    _usFusion.p_sliceNumber.increment();
                    break;
                case tgt::KeyEvent::K_DOWN:
                    _usFusion.p_sliceNumber.decrement();
                    break;
            }
        }
    }

    const std::string AdvancedUsVis::getName() const {
        return "AdvancedUsVis";
    }

}