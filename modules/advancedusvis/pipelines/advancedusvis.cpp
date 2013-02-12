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
#include "core/tools/simplejobprocessor.h"
#include "core/tools/job.h"

namespace campvis {

    AdvancedUsVis::AdvancedUsVis()
        : DigraphVisualizationPipeline()
        , _camera("camera", "Camera")
        , _usReader()
        , _confidenceReader()
        , _confidenceGenerator()
        , _gvg()
        , _lhh()
        , _usFusion1(_effectiveRenderTargetSize)
        , _usFusion2(_effectiveRenderTargetSize)
        , _usFusion3(_effectiveRenderTargetSize)
        , _usFusion4(_effectiveRenderTargetSize)
        , _usBlurFilter()
        , _quadView(_effectiveRenderTargetSize)
        , _usDenoiseilter()
        , _usProxy()
        , _usEEP(_effectiveRenderTargetSize)
        , _usDVR(_effectiveRenderTargetSize)
        , _wheelHandler(&_usFusion1.p_sliceNumber)
        , _tfWindowingHandler(&_usFusion1.p_transferFunction)
        , _trackballEH(0)
    {
        addProcessor(&_usReader);
        addProcessor(&_confidenceReader);
        //addProcessor(&_confidenceGenerator);
        addProcessor(&_gvg);
        //addProcessor(&_lhh);
        addProcessor(&_usBlurFilter);
        addProcessor(&_usFusion1);
        addProcessor(&_usFusion2);
        addProcessor(&_usFusion3);
        addProcessor(&_usFusion4);
        addProcessor(&_quadView);
        //addProcessor(&_usDenoiseilter);
        //addProcessor(&_usProxy);
        //addProcessor(&_usEEP);
        //addProcessor(&_usDVR);

        addEventHandler(&_wheelHandler);
        //addEventHandler(&_tfWindowingHandler);

        _trackballEH = new TrackballNavigationEventHandler(this, &_camera, _canvasSize.getValue());
        _eventHandlers.push_back(_trackballEH);
    }

    AdvancedUsVis::~AdvancedUsVis() {
        delete _trackballEH;
    }

    void AdvancedUsVis::init() {
        VisualizationPipeline::init();

        _usReader.s_validated.connect(this, &AdvancedUsVis::onProcessorValidated);

        _camera.addSharedProperty(&_usEEP.p_camera);
        _camera.addSharedProperty(&_usDVR.p_camera);

        _usReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\CurefabCS\\Stent_Patient_ B-Mode_2013-02-11T14.56.46z\\01_us.mhd");
        //_usReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\01\\BMode_01.mhd");
        //_usReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\UltrasoundBoneData\\SynthesEvaluationUnterschenkel\\Athanasios\\US.csvd");
        //_usReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\transcranial\\us.png");
        _usReader.p_targetImageID.setValue("us.image");
        _usReader.p_targetImageID.connect(&_confidenceGenerator.p_sourceImageID);
        _usReader.p_targetImageID.connect(&_usFusion1.p_usImageId);
        _usReader.p_targetImageID.connect(&_usFusion2.p_usImageId);
        _usReader.p_targetImageID.connect(&_usFusion3.p_usImageId);
        _usReader.p_targetImageID.connect(&_usFusion4.p_usImageId);
        _usReader.p_targetImageID.connect(&_gvg.p_sourceImageID);
        _usReader.p_targetImageID.connect(&_lhh.p_intensitiesId);
        _usReader.p_targetImageID.connect(&_usBlurFilter.p_sourceImageID);
        _usReader.p_targetImageID.connect(&_usDenoiseilter.p_sourceImageID);

        _confidenceReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\CurefabCS\\Stent_Patient_ B-Mode_2013-02-11T14.56.46z\\01_cm.mhd");
        //_confidenceReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\01\\Confidence_01.mhd");
        //_confidenceReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\UltrasoundBoneData\\SynthesEvaluationUnterschenkel\\Athanasios\\Map.csvd");
        //_confidenceReader.p_url.setValue("D:\\Medical Data\\US Confidence Vis\\transcranial\\cm.png");
        _confidenceReader.p_targetImageID.setValue("confidence.image.read");
        _confidenceReader.p_targetImageID.connect(&_usFusion1.p_confidenceImageID);
        _confidenceReader.p_targetImageID.connect(&_usFusion2.p_confidenceImageID);
        _confidenceReader.p_targetImageID.connect(&_usFusion3.p_confidenceImageID);
        _confidenceReader.p_targetImageID.connect(&_usFusion4.p_confidenceImageID);

        _confidenceGenerator.p_targetImageID.setValue("confidence.image.generated");
        //_confidenceGenerator.p_targetImageID.connect(&_usFusion1.p_confidenceImageID);
        //_confidenceGenerator.p_targetImageID.connect(&_usFusion2.p_confidenceImageID);
        //_confidenceGenerator.p_targetImageID.connect(&_usFusion3.p_confidenceImageID);
        //_confidenceGenerator.p_targetImageID.connect(&_usFusion4.p_confidenceImageID);

        _gvg.p_targetImageID.connect(&_lhh.p_gradientsId);
        _gvg.p_targetImageID.connect(&_usFusion1.p_gradientImageID);
        _gvg.p_targetImageID.connect(&_usFusion2.p_gradientImageID);
        _gvg.p_targetImageID.connect(&_usFusion3.p_gradientImageID);
        _gvg.p_targetImageID.connect(&_usFusion4.p_gradientImageID);

        _usFusion1.p_targetImageID.setValue("us.fused1");
        _usFusion1.p_targetImageID.connect(&_quadView.p_inputImage1);
        _usFusion1.p_view.selectById("us");
        _usFusion1.p_sliceNumber.setValue(0);
        _usFusion1.p_sliceNumber.addSharedProperty(&_usFusion2.p_sliceNumber);
        _usFusion1.p_sliceNumber.addSharedProperty(&_usFusion3.p_sliceNumber);
        _usFusion1.p_sliceNumber.addSharedProperty(&_usFusion4.p_sliceNumber);

        _usFusion2.p_targetImageID.setValue("us.fused2");
        _usFusion2.p_targetImageID.connect(&_quadView.p_inputImage2);
        _usFusion2.p_view.selectById("mappingSaturationHSV");

        _usFusion3.p_targetImageID.setValue("us.fused3");
        _usFusion3.p_targetImageID.connect(&_quadView.p_inputImage3);
        _usFusion3.p_view.selectById("mappingChromacityHCY");

        _usFusion4.p_targetImageID.setValue("us.fused4");
        _usFusion4.p_targetImageID.connect(&_quadView.p_inputImage4);
        _usFusion4.p_view.selectById("mappingSharpness");

        _usBlurFilter.p_targetImageID.setValue("us.blurred");
        _usBlurFilter.p_targetImageID.connect(&_usFusion1.p_blurredImageId);
        _usBlurFilter.p_targetImageID.connect(&_usFusion2.p_blurredImageId);
        _usBlurFilter.p_targetImageID.connect(&_usFusion3.p_blurredImageId);
        _usBlurFilter.p_targetImageID.connect(&_usFusion4.p_blurredImageId);
        _usBlurFilter.p_filterMode.selectById("gauss");
        _usBlurFilter.p_sigma.setValue(4.f);

        _quadView.p_outputImage.setValue("quadview.output");

        _usDenoiseilter.p_targetImageID.setValue("us.denoised");
        _usDenoiseilter.p_targetImageID.connect(&_usProxy.p_sourceImageID);
        _usDenoiseilter.p_targetImageID.connect(&_usEEP.p_sourceImageID);
        _usDenoiseilter.p_targetImageID.connect(&_usDVR.p_sourceImageID);
        _usDenoiseilter.p_filterMode.selectById("gradientDiffusion");
        _usDenoiseilter.p_numberOfSteps.setValue(3);

        _usProxy.p_geometryID.setValue("us.proxy");
        _usProxy.p_geometryID.connect(&_usEEP.p_geometryID);

        _usEEP.p_entryImageID.setValue("us.entry");
        _usEEP.p_entryImageID.connect(&_usDVR.p_entryImageID);
        _usEEP.p_exitImageID.setValue("us.exit");
        _usEEP.p_exitImageID.connect(&_usDVR.p_exitImageID);

        // TODO: replace this hardcoded domain by automatically determined from image min/max values
        Geometry1DTransferFunction* tf = new Geometry1DTransferFunction(128, tgt::vec2(0.f, 1.f));
        tf->addGeometry(TFGeometry1D::createQuad(tgt::vec2(0.f, 1.f), tgt::col4(0, 0, 0, 255), tgt::col4(255, 255, 255, 255)));
        _usFusion1.p_transferFunction.replaceTF(tf);
        _usFusion2.p_transferFunction.replaceTF(tf->clone());
        _usFusion3.p_transferFunction.replaceTF(tf->clone());
        _usFusion4.p_transferFunction.replaceTF(tf->clone());

        // TODO: replace this hardcoded domain by automatically determined from image min/max values
        Geometry1DTransferFunction* tf2 = new Geometry1DTransferFunction(256, tgt::vec2(0.f, 1.f));
        tf2->addGeometry(TFGeometry1D::createQuad(tgt::vec2(0.f, 1.f), tgt::col4(0, 0, 0, 255), tgt::col4(255, 255, 255, 255)));
        _usDVR.p_transferFunction.replaceTF(tf2);
        _usDVR.p_targetImageID.setValue("us.dvr");

        _renderTargetID.setValue("quadview.output");

        _trackballEH->setViewportSize(_effectiveRenderTargetSize.getValue());
        _effectiveRenderTargetSize.s_changed.connect<AdvancedUsVis>(this, &AdvancedUsVis::onRenderTargetSizeChanged);
    }

    void AdvancedUsVis::deinit() {
        _effectiveRenderTargetSize.s_changed.disconnect(this);
        VisualizationPipeline::deinit();
    }

    void AdvancedUsVis::execute() {
/*
        if (!_usReader.getInvalidationLevel().isValid()) {
            SimpleJobProc.enqueueJob(makeJob(this, &AdvancedUsVis::foobar));
        }
        if (!_usDenoiseilter.getInvalidationLevel().isValid()) {
            SimpleJobProc.enqueueJob(makeJob<AdvancedUsVis, AbstractProcessor*>(this, &AdvancedUsVis::executeProcessor, &_usDenoiseilter));
        }
        if (!_confidenceGenerator.getInvalidationLevel().isValid()) {
            SimpleJobProc.enqueueJob(makeJob<AdvancedUsVis, AbstractProcessor*>(this, &AdvancedUsVis::executeProcessor, &_confidenceGenerator));
        }


        for (std::vector<AbstractProcessor*>::iterator it = _processors.begin(); it != _processors.end(); ++it) {
            if (! (*it)->getInvalidationLevel().isValid())
                lockGLContextAndExecuteProcessor(*it);
        }*/
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
            }
        }
    }

    const std::string AdvancedUsVis::getName() const {
        return "AdvancedUsVis";
    }

    void AdvancedUsVis::onRenderTargetSizeChanged(const AbstractProperty* prop) {
        _trackballEH->setViewportSize(_canvasSize.getValue());
        float ratio = static_cast<float>(_effectiveRenderTargetSize.getValue().x) / static_cast<float>(_effectiveRenderTargetSize.getValue().y);
        _camera.setWindowRatio(ratio);
    }

    void AdvancedUsVis::onProcessorValidated(AbstractProcessor* processor) {
        if (processor = &_usReader) {
            // convert data
            DataContainer::ScopedTypedData<ImageData> img(_data, _usReader.p_targetImageID.getValue());
            if (img != 0) {
                tgt::Bounds volumeExtent = img->getWorldBounds();
                tgt::vec3 pos = volumeExtent.center() - tgt::vec3(0, 0, tgt::length(volumeExtent.diagonal()));

                _trackballEH->setSceneBounds(volumeExtent);
                _trackballEH->setCenter(volumeExtent.center());
                _trackballEH->reinitializeCamera(pos, volumeExtent.center(), _camera.getValue().getUpVector());
            }

        }
    }

}