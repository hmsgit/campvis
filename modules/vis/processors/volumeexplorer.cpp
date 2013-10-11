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

#include "volumeexplorer.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/facegeometry.h"
#include "core/datastructures/renderdata.h"

#include "core/classification/simpletransferfunction.h"
#include "core/tools/quadrenderer.h"

namespace campvis {
    const std::string VolumeExplorer::loggerCat_ = "CAMPVis.modules.vis.VolumeExplorer";

    VolumeExplorer::VolumeExplorer(IVec2Property* viewportSizeProp)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputVolume("InputVolume", "Input Volume", "", DataNameProperty::READ, AbstractProcessor::INVALID_PROPERTIES)
        , p_outputImage("OutputImage", "Output Image", "ve.output", DataNameProperty::WRITE)
        , p_seProperties("SliceExtractorProperties", "Slice Extractor Properties", AbstractProcessor::VALID)
        , p_vrProperties("VolumeRendererProperties", "Volume Renderer Properties", AbstractProcessor::VALID)
        , _raycaster(viewportSizeProp)
        , _sliceExtractor(viewportSizeProp)
        , p_sliceRenderSize("SliceRenderSize", "Slice Render Size", tgt::ivec2(32), tgt::ivec2(0), tgt::ivec2(10000), tgt::ivec2(1), AbstractProcessor::VALID)
        , p_volumeRenderSize("VolumeRenderSize", "Volume Render Size", tgt::ivec2(32), tgt::ivec2(0), tgt::ivec2(10000), tgt::ivec2(1), AbstractProcessor::VALID)
        , _xSliceHandler(&_sliceExtractor.p_xSliceNumber)
        , _ySliceHandler(&_sliceExtractor.p_ySliceNumber)
        , _zSliceHandler(&_sliceExtractor.p_zSliceNumber)
        , _windowingHandler(&_sliceExtractor.p_transferFunction)
        , _trackballEH(0)
        , _mousePressed(false)
    {
        addProperty(&p_inputVolume);
        addProperty(&p_outputImage);

        p_seProperties.addPropertyCollection(_sliceExtractor);
        _sliceExtractor.p_lqMode.setVisible(false);
        _sliceExtractor.p_sourceImageID.setVisible(false);
        _sliceExtractor.p_targetImageID.setVisible(false);
        _sliceExtractor.p_sliceOrientation.setVisible(false);
        _sliceExtractor.p_xSliceColor.setVisible(false);
        _sliceExtractor.p_ySliceColor.setVisible(false);
        _sliceExtractor.p_zSliceColor.setVisible(false);
        addProperty(&p_seProperties);

        p_vrProperties.addPropertyCollection(_raycaster);
        _raycaster.p_lqMode.setVisible(false);
        _raycaster.p_inputVolume.setVisible(false);
        _raycaster.p_outputImage.setVisible(false);
        addProperty(&p_vrProperties);

        addProperty(&_sliceExtractor.p_transferFunction);
        addProperty(_raycaster.getProperty("TransferFunction"));

        p_inputVolume.addSharedProperty(&_raycaster.p_inputVolume);
        p_inputVolume.addSharedProperty(&_sliceExtractor.p_sourceImageID);

        _sliceExtractor.setViewportSizeProperty(&p_sliceRenderSize);
        _raycaster.setViewportSizeProperty(&p_volumeRenderSize);

        addProperty(&p_sliceRenderSize);
        addProperty(&p_volumeRenderSize);


        // Event-Handlers
        _trackballEH = new TrackballNavigationEventListener(&_raycaster.p_camera, &p_volumeRenderSize);
        _trackballEH->addLqModeProcessor(&_raycaster);
    }

    VolumeExplorer::~VolumeExplorer() {
        delete _trackballEH;
    }

    void VolumeExplorer::init() {
        VisualizationProcessor::init();
        _raycaster.init();
        _sliceExtractor.init();

        _shader = ShdrMgr.loadSeparate("core/glsl/passthrough.vert", "modules/vis/glsl/quadview.frag", "", false);
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");

        _sliceExtractor.s_invalidated.connect(this, &VolumeExplorer::onProcessorInvalidated);
        _raycaster.s_invalidated.connect(this, &VolumeExplorer::onProcessorInvalidated);

        std::vector<tgt::vec3> vertices, texCorods;

        vertices.push_back(tgt::vec3( 0.f,  0.f, 0.f));
        vertices.push_back(tgt::vec3( 1.f,  0.f, 0.f));
        vertices.push_back(tgt::vec3( 1.f,  1.f, 0.f));
        vertices.push_back(tgt::vec3( 0.f,  1.f, 0.f));
        texCorods.push_back(tgt::vec3(0.f, 0.f, 0.f));
        texCorods.push_back(tgt::vec3(1.f, 0.f, 0.f));
        texCorods.push_back(tgt::vec3(1.f, 1.f, 0.f));
        texCorods.push_back(tgt::vec3(0.f, 1.f, 0.f));

        _quad = new FaceGeometry(vertices, texCorods);
        _quad->createGLBuffers();
    }

    void VolumeExplorer::deinit() {
        _raycaster.deinit();
        _sliceExtractor.deinit();
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
        delete _quad;
    }

    void VolumeExplorer::process(DataContainer& data) {
        // launch sub-renderers if necessary
        if (getInvalidationLevel() & VR_INVALID) {
            _raycaster.process(data);
        }
        if (getInvalidationLevel() & SLICES_INVALID) {
            _sliceExtractor.p_sliceOrientation.selectById("x");
            _sliceExtractor.p_targetImageID.setValue(p_outputImage.getValue() + ".xSlice");
            _sliceExtractor.process(data);

            _sliceExtractor.p_sliceOrientation.selectById("y");
            _sliceExtractor.p_targetImageID.setValue(p_outputImage.getValue() + ".ySlice");
            _sliceExtractor.process(data);

            _sliceExtractor.p_sliceOrientation.selectById("z");
            _sliceExtractor.p_targetImageID.setValue(p_outputImage.getValue() + ".zSlice");
            _sliceExtractor.process(data);
        }

        // compose rendering
        composeFinalRendering(data);

        validate(INVALID_RESULT | VR_INVALID | SLICES_INVALID);
    }

    void VolumeExplorer::onPropertyChanged(const AbstractProperty* prop) {
        if (prop == _viewportSizeProperty) {
            p_sliceRenderSize.setValue(tgt::ivec2(_viewportSizeProperty->getValue().y / 3, _viewportSizeProperty->getValue().y / 3));
            p_volumeRenderSize.setValue(tgt::ivec2(_viewportSizeProperty->getValue().x - _viewportSizeProperty->getValue().y / 3, _viewportSizeProperty->getValue().y));
        }
        if (prop == &p_outputImage) {
            _raycaster.p_outputImage.setValue(p_outputImage.getValue() + ".raycaster");
        }
        if (prop == &p_inputVolume) {
            invalidate(SLICES_INVALID);
        }
        VisualizationProcessor::onPropertyChanged(prop);
    }

    void VolumeExplorer::composeFinalRendering(DataContainer& data) {
        ScopedTypedData<RenderData> vrImage(data, p_outputImage.getValue() + ".raycaster");
        ScopedTypedData<RenderData> xSliceImage(data, p_outputImage.getValue() + ".xSlice");
        ScopedTypedData<RenderData> ySliceImage(data, p_outputImage.getValue() + ".ySlice");
        ScopedTypedData<RenderData> zSliceImage(data, p_outputImage.getValue() + ".zSlice");

        if (vrImage == 0 && xSliceImage == 0 && ySliceImage == 0 && zSliceImage == 0)
            return;

        FramebufferActivationGuard fag(this);
        createAndAttachColorTexture();
        createAndAttachDepthTexture();

        tgt::TextureUnit colorUnit, depthUnit;
        _shader->activate();

        tgt::vec2 rts(_viewportSizeProperty->getValue());
        tgt::vec2 vrs(p_volumeRenderSize.getValue());
        tgt::vec2 srs(p_sliceRenderSize.getValue());

        _shader->setUniform("_projectionMatrix", tgt::mat4::createOrtho(0, rts.x, rts.y, 0, -1, 1));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (vrImage != 0) {
            vrImage->bind(_shader, colorUnit, depthUnit);
            float ratio = static_cast<float>(vrs.x) / static_cast<float>(rts.x);
            _shader->setUniform("_modelMatrix", tgt::mat4::createScale(tgt::vec3(vrs.x, vrs.y, .5f)));
            _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(srs.x, 0.f, 0.f)));
            _quad->render(GL_POLYGON);
        }
        if (xSliceImage != 0) {
            xSliceImage->bind(_shader, colorUnit, depthUnit);
            _shader->setUniform("_modelMatrix", tgt::mat4::createScale(tgt::vec3(srs.x, srs.y, .5f)));
            _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(0.f, 2.f * srs.y, 0.f)));
            _quad->render(GL_POLYGON);
        }
        if (ySliceImage != 0) {
            ySliceImage->bind(_shader, colorUnit, depthUnit);
            _shader->setUniform("_modelMatrix", tgt::mat4::createScale(tgt::vec3(srs.x, srs.y, .5f)));
            _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(0.f, srs.y, 0.f)));
            _quad->render(GL_POLYGON);
        }
        if (zSliceImage != 0) {
            zSliceImage->bind(_shader, colorUnit, depthUnit);
            _shader->setUniform("_modelMatrix", tgt::mat4::createScale(tgt::vec3(srs.x, srs.y, .5f)));
            _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(0.f, 0.f, 0.f)));
            _quad->render(GL_POLYGON);
        }

        _shader->deactivate();
        tgt::TextureUnit::setZeroUnit();
        LGL_ERROR;

        data.addData(p_outputImage.getValue(), new RenderData(_fbo));
    }

    void VolumeExplorer::onProcessorInvalidated(AbstractProcessor* processor) {
        // make sure to only invalidate ourself if the invalidation is not triggered by us
        // => the _locked state is a trustworthy source for this information :)
        if (! isLocked()) {
            if (processor == &_raycaster) {
                invalidate(VR_INVALID);
            }
            if (processor == &_sliceExtractor) {
                invalidate(SLICES_INVALID);
            }

            invalidate(AbstractProcessor::INVALID_RESULT);
        }
    }

    void VolumeExplorer::updateProperties(DataContainer& dc) {
        ScopedTypedData<ImageData> img(dc, p_inputVolume.getValue());
        _sliceExtractor.p_transferFunction.getTF()->setImageHandle(img.getDataHandle());
        static_cast<TransferFunctionProperty*>(_raycaster.getProperty("TransferFunction"))->getTF()->setImageHandle(img.getDataHandle());

        if (img != 0) {
            const tgt::svec3& imgSize = img->getSize();
            if (_sliceExtractor.p_xSliceNumber.getMaxValue() != imgSize.x - 1){
                _sliceExtractor.p_xSliceNumber.setMaxValue(static_cast<int>(imgSize.x) - 1);
                _sliceExtractor.p_xSliceNumber.setValue(static_cast<int>(imgSize.x) / 2);
            }
            if (_sliceExtractor.p_ySliceNumber.getMaxValue() != imgSize.y - 1){
                _sliceExtractor.p_ySliceNumber.setMaxValue(static_cast<int>(imgSize.y) - 1);
                _sliceExtractor.p_ySliceNumber.setValue(static_cast<int>(imgSize.y) / 2);
            }
            if (_sliceExtractor.p_zSliceNumber.getMaxValue() != imgSize.z - 1){
                _sliceExtractor.p_zSliceNumber.setMaxValue(static_cast<int>(imgSize.z) - 1);
                _sliceExtractor.p_zSliceNumber.setValue(static_cast<int>(imgSize.z) / 2);
            }

            _trackballEH->reinitializeCamera(img);
        }

        validate(AbstractProcessor::INVALID_PROPERTIES);
    }

    void VolumeExplorer::onEvent(tgt::Event* e) {
        // forward the event to the correspsonding event listeners depending on the mouse position
        if (typeid(*e) == typeid(tgt::MouseEvent)) {
            tgt::MouseEvent* me = static_cast<tgt::MouseEvent*>(e);

            if (!_mousePressed && me->x() <= p_sliceRenderSize.getValue().x) {
                // cycle slices
                if (me->action() == tgt::MouseEvent::WHEEL) {
                    if (me->y() <= p_sliceRenderSize.getValue().y)
                        _xSliceHandler.onEvent(e);
                    else if (me->y() <= 2*p_sliceRenderSize.getValue().y)
                        _ySliceHandler.onEvent(e);
                    else
                        _zSliceHandler.onEvent(e);
                }

                // adjust slice TF windowing
                else {
                    _windowingHandler.onEvent(e);
                }
            }
            else {
                // raycasting trackball navigation
                if (me->action() == tgt::MouseEvent::PRESSED)
                    _mousePressed = true;
                else if (me->action() == tgt::MouseEvent::RELEASED)
                    _mousePressed = false;

                tgt::MouseEvent adjustedMe(
                    me->x() - p_sliceRenderSize.getValue().x,
                    me->y(),
                    me->action(),
                    me->modifiers(),
                    me->button(),
                    me->viewport() - tgt::ivec2(p_sliceRenderSize.getValue().x, 0)
                    );
                _trackballEH->onEvent(&adjustedMe);
            }
        }
    }

}

