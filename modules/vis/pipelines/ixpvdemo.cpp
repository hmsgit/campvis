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

#include "ixpvdemo.h"

#include "tgt/event/keyevent.h"
#include "core/datastructures/imagedataconverter.h"
#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    IxpvDemo::IxpvDemo()
        : VisualizationPipeline()
        , _xrayReader()
        , _ctReader()
        , _usReader()
        , _usSliceExtractor(_effectiveRenderTargetSize)
        , _wheelHandler(&_usSliceExtractor.p_sliceNumber)
        , _tfWindowingHandler(&_usSliceExtractor.p_transferFunction)
    {
        addProcessor(&_xrayReader);
        addProcessor(&_ctReader);
        addProcessor(&_usReader);
        addProcessor(&_usSliceExtractor);
        addEventHandler(&_wheelHandler);
        addEventHandler(&_tfWindowingHandler);
    }

    IxpvDemo::~IxpvDemo() {
    }

    void IxpvDemo::init() {
        VisualizationPipeline::init();

        _xrayReader.p_url.setValue("D:\\Medical Data\\XrayDepthPerception\\DataCowLeg\\Cowleg_CarmXrayImages\\APView_1.jpg");
        _xrayReader.p_targetImageID.setValue("xray");

        _usSliceExtractor.p_sliceNumber.setValue(0);

        // TODO: replace this hardcoded domain by automatically determined from image min/max values
        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(128, tgt::vec2(0.f, .08f));
        tf->addGeometry(TFGeometry1D::createQuad(tgt::vec2(0.f, 1.f), tgt::col4(0, 0, 0, 0), tgt::col4(255, 255, 255, 255)));
        _usSliceExtractor.p_transferFunction.replaceTF(tf);

        _renderTargetID.setValue("renderTarget");
        _renderTargetID.addSharedProperty(&(_usSliceExtractor.p_targetImageID));
    }

    void IxpvDemo::execute() {
        {
            tbb::spin_mutex::scoped_lock lock(_localMutex);
            _invalidationLevel.setValid();
            // TODO:    think whether we want to lock all processors already here.
        }
        if (! _xrayReader.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(&_xrayReader);

            // convert data
            DataContainer::ScopedTypedData<ImageData> img(_data, _xrayReader.p_targetImageID.getValue());
            ImageDataLocal* local = ImageDataConverter::tryConvert<ImageDataLocal>(img);
            if (local != 0) {
                DataHandle dh = _data.addData("se.input", local);
                _usSliceExtractor.p_transferFunction.getTF()->setImageHandle(dh);
            }
        }
        if (! _usSliceExtractor.getInvalidationLevel().isValid()) {
            lockGLContextAndExecuteProcessor(&_usSliceExtractor);
        }
    }

    void IxpvDemo::keyEvent(tgt::KeyEvent* e) {
        if (e->pressed()) {
        }
    }

    const std::string IxpvDemo::getName() const {
        return "IXPV Demo";
    }

}