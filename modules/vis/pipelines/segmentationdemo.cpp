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

#include "segmentationdemo.h"

#include "tgt/event/keyevent.h"
#include "core/datastructures/imagedata.h"

#include "core/classification/geometry1dtransferfunction.h"
#include "core/classification/tfgeometry1d.h"

namespace campvis {

    SegmentationDemo::SegmentationDemo(DataContainer* dc)
        : AutoEvaluationPipeline(dc)
        , _lsp()
        , _imageReader()
        , _itkFilter()
        , _itkSegmentation(&_canvasSize)
    {
        addProcessor(&_lsp);
        addProcessor(&_imageReader);
        addProcessor(&_itkFilter);
        addProcessor(&_itkSegmentation);

        addEventListenerToBack(&_itkSegmentation);
    }

    SegmentationDemo::~SegmentationDemo() {
    }

    void SegmentationDemo::init() {
        AutoEvaluationPipeline::init();

        _itkSegmentation.p_outputImage.setValue("result");
        _renderTargetID.setValue("result");

        //_imageReader.setURL(CAMPVIS_SOURCE_DIR "/modules/vis/sampledata/smallHeart.mhd");
        _imageReader.setURL(CAMPVIS_SOURCE_DIR "/../misc/mha_loader_CAMPVis_volumes/prostate_phantom_US/prostate_phantom_fcal_volume_uncompressed.mha");
        _imageReader.setTargetImageId("reader.output");
        _imageReader.setTargetImageIdSharedProperty(&_itkSegmentation.p_inputVolume);


        Geometry1DTransferFunction* dvrTF = new Geometry1DTransferFunction(128, tgt::vec2(0.f, .05f));
        dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.4f, .5f), tgt::col4(32, 192, 0, 128), tgt::col4(32, 192, 0, 128)));
        dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.12f, .15f), tgt::col4(85, 0, 0, 128), tgt::col4(255, 0, 0, 128)));
        dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.19f, .28f), tgt::col4(89, 89, 89, 155), tgt::col4(89, 89, 89, 155)));
        dvrTF->addGeometry(TFGeometry1D::createQuad(tgt::vec2(.41f, .51f), tgt::col4(170, 170, 128, 64), tgt::col4(192, 192, 128, 64)));

        static_cast<TransferFunctionProperty*>(_itkSegmentation.getNestedProperty("VolumeRendererProperties::RaycasterProps::TransferFunction"))->replaceTF(dvrTF);
        static_cast<FloatProperty*>(_itkSegmentation.getNestedProperty("VolumeRendererProperties::RaycasterProps::SamplingRate"))->setValue(4.f);
    }

    void SegmentationDemo::deinit() {
        _canvasSize.s_changed.disconnect(this);
        AutoEvaluationPipeline::deinit();
    }
}