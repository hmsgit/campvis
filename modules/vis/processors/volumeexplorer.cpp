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

#include "volumeexplorer.h"
#include "tgt/logmanager.h"
#include "tgt/shadermanager.h"
#include "tgt/textureunit.h"

#include "core/datastructures/facegeometry.h"
#include "core/datastructures/geometrydatafactory.h"
#include "core/datastructures/renderdata.h"

#include "core/classification/simpletransferfunction.h"
#include "core/pipeline/processordecoratorbackground.h"
#include "core/tools/quadrenderer.h"

namespace campvis {
    static const GenericOption<VolumeExplorer::Views> largeViewOptions[4] = {
        GenericOption<VolumeExplorer::Views>("z", "XY Plane", VolumeExplorer::XY_PLANE),
        GenericOption<VolumeExplorer::Views>("y", "XZ Plane", VolumeExplorer::XZ_PLANE),
        GenericOption<VolumeExplorer::Views>("x", "YZ Plane", VolumeExplorer::YZ_PLANE),
        GenericOption<VolumeExplorer::Views>("volume", "Volume", VolumeExplorer::VOLUME)
    };

    const std::string VolumeExplorer::loggerCat_ = "CAMPVis.modules.vis.VolumeExplorer";

    VolumeExplorer::VolumeExplorer(IVec2Property* viewportSizeProp, SliceRenderProcessor* sliceRenderer, RaycastingProcessor* raycaster)
        : VisualizationProcessor(viewportSizeProp)
        , p_inputVolume("InputVolume", "Input Volume", "", DataNameProperty::READ)
        , p_outputImage("OutputImage", "Output Image", "ve.output", DataNameProperty::WRITE)
        , p_largeView("LargeView", "Large View Selection", largeViewOptions, 4)
        , p_enableScribbling("EnableScribbling", "Enable Scribbling in Slice Views", false)
        , p_seProperties("SliceExtractorProperties", "Slice Extractor Properties")
        , p_vrProperties("VolumeRendererProperties", "Volume Renderer Properties")
        , _raycaster(viewportSizeProp, raycaster)
        , _sliceRenderer(sliceRenderer)
        , p_smallRenderSize("SmallRenderSize", "Small Render Size", tgt::ivec2(32), tgt::ivec2(0), tgt::ivec2(10000), tgt::ivec2(1))
        , p_largeRenderSize("LargeRenderSize", "Large Render Size", tgt::ivec2(32), tgt::ivec2(0), tgt::ivec2(10000), tgt::ivec2(1))
        , _xSliceHandler(&_sliceRenderer->p_xSliceNumber)
        , _ySliceHandler(&_sliceRenderer->p_ySliceNumber)
        , _zSliceHandler(&_sliceRenderer->p_zSliceNumber)
        , _windowingHandler(nullptr)
        , _trackballEH(0)
        , _mousePressedInRaycaster(false)
        , _scribblePointer(nullptr)
        , _cachedImageSize(0)
    {
        tgtAssert(raycaster != nullptr, "Raycasting Processor must not be 0.");
        tgtAssert(_sliceRenderer != nullptr, "Slice Rendering Processor must not be 0.");

        p_largeView.selectByOption(VOLUME);

        addProperty(p_inputVolume, INVALID_PROPERTIES);
        addProperty(p_outputImage);
        addProperty(p_largeView, LARGE_VIEW_INVALID | SLICES_INVALID | VR_INVALID | INVALID_RESULT);
        addProperty(p_enableScribbling, VALID);

        addDecorator(new ProcessorDecoratorBackground());
        decoratePropertyCollection(this);
        
        p_seProperties.addPropertyCollection(*_sliceRenderer);
        _sliceRenderer->p_lqMode.setVisible(false);
        _sliceRenderer->p_sourceImageID.setVisible(false);
        _sliceRenderer->p_targetImageID.setVisible(false);
        _sliceRenderer->p_sliceOrientation.setVisible(false);
        _sliceRenderer->p_xSliceColor.setVisible(false);
        _sliceRenderer->p_ySliceColor.setVisible(false);
        _sliceRenderer->p_zSliceColor.setVisible(false);
        addProperty(p_seProperties, VALID);

        p_vrProperties.addPropertyCollection(_raycaster);
        _raycaster.p_lqMode.setVisible(false);
        _raycaster.p_inputVolume.setVisible(false);
        _raycaster.p_outputImage.setVisible(false);
        addProperty(p_vrProperties, VALID);

        p_inputVolume.addSharedProperty(&_raycaster.p_inputVolume);
        p_inputVolume.addSharedProperty(&_sliceRenderer->p_sourceImageID);

        _sliceRenderer->setViewportSizeProperty(&p_smallRenderSize);
        _raycaster.setViewportSizeProperty(&p_largeRenderSize);

        addProperty(p_smallRenderSize, VALID);
        addProperty(p_largeRenderSize, VALID);

        // Event-Handlers
        _trackballEH = new TrackballNavigationEventListener(&_raycaster.p_camera, &p_largeRenderSize);
        _trackballEH->addLqModeProcessor(&_raycaster);

        if (TransferFunctionProperty* tester = dynamic_cast<TransferFunctionProperty*>(_sliceRenderer->getProperty("TransferFunction"))) {
        	_windowingHandler.setTransferFunctionProperty(tester);
        }
    }

    VolumeExplorer::~VolumeExplorer() {
        delete _trackballEH;
    }

    void VolumeExplorer::init() {
        VisualizationProcessor::init();
        _raycaster.init();
        _sliceRenderer->init();

        _shader = ShdrMgr.load("core/glsl/passthrough.vert", "modules/vis/glsl/volumeexplorer.frag", "");
        _shader->setAttributeLocation(0, "in_Position");
        _shader->setAttributeLocation(1, "in_TexCoord");

        _sliceRenderer->s_invalidated.connect(this, &VolumeExplorer::onProcessorInvalidated);
        _raycaster.s_invalidated.connect(this, &VolumeExplorer::onProcessorInvalidated);

        _quad = GeometryDataFactory::createQuad(tgt::vec3(0.f), tgt::vec3(1.f), tgt::vec3(0.f), tgt::vec3(1.f));
        
        // force recalculation of p_smallRenderSize and p_largeRenderSize
        onPropertyChanged(_viewportSizeProperty);
    }

    void VolumeExplorer::deinit() {
        _raycaster.deinit();
        _sliceRenderer->deinit();
        VisualizationProcessor::deinit();
        ShdrMgr.dispose(_shader);
        delete _quad;
    }

    void VolumeExplorer::updateResult(DataContainer& data) {
        // launch sub-renderers if necessary
        if (getInvalidationLevel() & LARGE_VIEW_INVALID) {
            switch (p_largeView.getOptionValue()) {
                case XY_PLANE: // fallthrough
                case XZ_PLANE: // fallthrough
                case YZ_PLANE:
                    _raycaster.setViewportSizeProperty(&p_smallRenderSize);
                    _trackballEH->setViewportSizeProperty(&p_smallRenderSize);
                    break;
                case VOLUME:
                    _raycaster.setViewportSizeProperty(&p_largeRenderSize);
                    _trackballEH->setViewportSizeProperty(&p_largeRenderSize);
                    break;
            }
            validate(LARGE_VIEW_INVALID);
        }
        if (getInvalidationLevel() & SCRIBBLE_INVALID) {
            std::vector<tgt::vec3> vertices;
            std::vector<tgt::vec4> colors;

            for (size_t i = 0; i < _yesScribbles.size(); ++i) {
                vertices.push_back(tgt::vec3(_yesScribbles[i]));
                colors.push_back(tgt::vec4(.2f, .8f, 0.f, 1.f));
            }
            for (size_t i = 0; i < _noScribbles.size(); ++i) {
                vertices.push_back(tgt::vec3(_noScribbles[i]));
                colors.push_back(tgt::vec4(.85f, .2f, 0.f, 1.f));
            }

            FaceGeometry* g = new FaceGeometry(vertices, std::vector<tgt::vec3>(), colors);
            data.addData(p_outputImage.getValue() + ".scribbles", g);
            validate(SCRIBBLE_INVALID);

            // force update of slice renderer if necessary
            if (! (getInvalidationLevel() & VR_INVALID))
                invalidate(SLICES_INVALID);
        }
        if (getInvalidationLevel() & VR_INVALID) {
            _raycaster.process(data);
        }
        if (getInvalidationLevel() & SLICES_INVALID) {
            _sliceRenderer->setViewportSizeProperty(p_largeView.getOptionValue() == YZ_PLANE ? &p_largeRenderSize : &p_smallRenderSize);
            _sliceRenderer->p_sliceOrientation.selectByOption(SliceRenderProcessor::YZ_PLANE);
            _sliceRenderer->p_targetImageID.setValue(p_outputImage.getValue() + ".xSlice");
            _sliceRenderer->process(data);

            _sliceRenderer->setViewportSizeProperty(p_largeView.getOptionValue() == XZ_PLANE ? &p_largeRenderSize : &p_smallRenderSize);
            _sliceRenderer->p_sliceOrientation.selectByOption(SliceRenderProcessor::XZ_PLANE);
            _sliceRenderer->p_targetImageID.setValue(p_outputImage.getValue() + ".ySlice");
            _sliceRenderer->process(data);

            _sliceRenderer->setViewportSizeProperty(p_largeView.getOptionValue() == XY_PLANE ? &p_largeRenderSize : &p_smallRenderSize);
            _sliceRenderer->p_sliceOrientation.selectByOption(SliceRenderProcessor::XY_PLANE);
            _sliceRenderer->p_targetImageID.setValue(p_outputImage.getValue() + ".zSlice");
            _sliceRenderer->process(data);
        }

        // compose rendering
        composeFinalRendering(data);

        validate(INVALID_RESULT | VR_INVALID | SLICES_INVALID);
    }

    void VolumeExplorer::onPropertyChanged(const AbstractProperty* prop) {
        if (prop == _viewportSizeProperty) {
            p_smallRenderSize.setValue(tgt::ivec2(_viewportSizeProperty->getValue().y / 3, _viewportSizeProperty->getValue().y / 3));
            p_largeRenderSize.setValue(tgt::ivec2(_viewportSizeProperty->getValue().x - _viewportSizeProperty->getValue().y / 3, _viewportSizeProperty->getValue().y));
        }
        if (prop == &p_outputImage) {
            _raycaster.p_outputImage.setValue(p_outputImage.getValue() + ".raycaster");
            _sliceRenderer->p_geometryID.setValue(p_outputImage.getValue() + ".scribbles");
        }
        if (prop == &p_inputVolume) {
            invalidate(VR_INVALID | SLICES_INVALID);
        }
        if (prop == &p_enableScribbling) {
            if (p_enableScribbling.getValue() == true) {
                _sliceRenderer->s_scribblePainted.connect(this, &VolumeExplorer::onSliceExtractorScribblePainted);
                _sliceRenderer->p_geometryID.setValue(p_outputImage.getValue() + ".scribbles");
            }
            else {
                _sliceRenderer->s_scribblePainted.disconnect(this);
                _sliceRenderer->p_geometryID.setValue("");
            }
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
        tgt::vec2 vrs(p_largeRenderSize.getValue());
        tgt::vec2 srs(p_smallRenderSize.getValue());

        _shader->setUniform("_projectionMatrix", tgt::mat4::createOrtho(0, rts.x, rts.y, 0, -1, 1));
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (vrImage != 0) {
            decorateRenderProlog(data, _shader);
            _shader->setUniform("_renderBackground", true);

            vrImage->bind(_shader, colorUnit, depthUnit);
            switch (p_largeView.getOptionValue()) {
                case XY_PLANE:
                    _shader->setUniform("_modelMatrix", tgt::mat4::createScale(tgt::vec3(srs.x, srs.y, .5f)));
                    _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(0.f, 2.f * srs.y, 0.f)));
                    break;
                case XZ_PLANE:
                    _shader->setUniform("_modelMatrix", tgt::mat4::createScale(tgt::vec3(srs.x, srs.y, .5f)));
                    _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(0.f, srs.y, 0.f)));
                    break;
                case YZ_PLANE:
                    _shader->setUniform("_modelMatrix", tgt::mat4::createScale(tgt::vec3(srs.x, srs.y, .5f)));
                    _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(0.f, 0.f, 0.f)));
                    break;
                case VOLUME:
                    _shader->setUniform("_modelMatrix", tgt::mat4::createScale(tgt::vec3(vrs.x, vrs.y, .5f)));
                    _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(srs.x, 0.f, 0.f)));
                    break;
            }
            _quad->render(GL_POLYGON);

            _shader->setUniform("_renderBackground", false);
            decorateRenderEpilog(_shader);
        }
        if (zSliceImage != 0) {
            zSliceImage->bind(_shader, colorUnit, depthUnit);
            if (p_largeView.getOptionValue() == XY_PLANE) {
                _shader->setUniform("_modelMatrix", tgt::mat4::createScale(tgt::vec3(vrs.x, vrs.y, .5f)));
                _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(srs.x, 0.f, 0.f)));
            }
            else {
                _shader->setUniform("_modelMatrix", tgt::mat4::createScale(tgt::vec3(srs.x, srs.y, .5f)));
                _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(0.f, 2.f * srs.y, 0.f)));
            }
            _quad->render(GL_POLYGON);
        }
        if (ySliceImage != 0) {
            ySliceImage->bind(_shader, colorUnit, depthUnit);
            if (p_largeView.getOptionValue() == XZ_PLANE) {
                _shader->setUniform("_modelMatrix", tgt::mat4::createScale(tgt::vec3(vrs.x, vrs.y, .5f)));
                _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(srs.x, 0.f, 0.f)));
            }
            else {
                _shader->setUniform("_modelMatrix", tgt::mat4::createScale(tgt::vec3(srs.x, srs.y, .5f)));
                _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(0.f, srs.y, 0.f)));
            }
            _quad->render(GL_POLYGON);
        }
        if (xSliceImage != 0) {
            xSliceImage->bind(_shader, colorUnit, depthUnit);
            if (p_largeView.getOptionValue() == YZ_PLANE) {
                _shader->setUniform("_modelMatrix", tgt::mat4::createScale(tgt::vec3(vrs.x, vrs.y, .5f)));
                _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(srs.x, 0.f, 0.f)));
            }
            else {
                _shader->setUniform("_modelMatrix", tgt::mat4::createScale(tgt::vec3(srs.x, srs.y, .5f)));
                _shader->setUniform("_viewMatrix", tgt::mat4::createTranslation(tgt::vec3(0.f, 0.f, 0.f)));
            }
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
            if (processor == _sliceRenderer) {
                invalidate(SLICES_INVALID);
            }

            invalidate(AbstractProcessor::INVALID_RESULT);
        }
    }

    void VolumeExplorer::updateProperties(DataContainer& dc) {
        ScopedTypedData<ImageData> img(dc, p_inputVolume.getValue());
        static_cast<TransferFunctionProperty*>(_raycaster.getNestedProperty("RaycasterProps::TransferFunction"))->setImageHandle(img.getDataHandle());

        if (img != 0 && _cachedImageSize != tgt::ivec3(img->getSize())) {
            _cachedImageSize = img->getSize();
            if (_sliceRenderer->p_xSliceNumber.getMaxValue() != _cachedImageSize.x - 1){
                _sliceRenderer->p_xSliceNumber.setMaxValue(_cachedImageSize.x - 1);
                _sliceRenderer->p_xSliceNumber.setValue(_cachedImageSize.x / 2);
            }
            if (_sliceRenderer->p_ySliceNumber.getMaxValue() != _cachedImageSize.y - 1){
                _sliceRenderer->p_ySliceNumber.setMaxValue(_cachedImageSize.y - 1);
                _sliceRenderer->p_ySliceNumber.setValue(_cachedImageSize.y / 2);
            }
            if (_sliceRenderer->p_zSliceNumber.getMaxValue() != _cachedImageSize.z - 1){
                _sliceRenderer->p_zSliceNumber.setMaxValue(_cachedImageSize.z - 1);
                _sliceRenderer->p_zSliceNumber.setValue(_cachedImageSize.z / 2);
            }

            _trackballEH->reinitializeCamera(img);
        }
    }

    void VolumeExplorer::onEvent(tgt::Event* e) {
        // forward the event to the corresponding event listeners depending on the mouse position
        if (typeid(*e) == typeid(tgt::MouseEvent)) {
            tgt::MouseEvent* me = static_cast<tgt::MouseEvent*>(e);

            // if the mouse was pressed, we need to cache the view parameters of the view underneath
            // the pointer, so that we can adjust the MouseEvents to the corresponding subviews.
            if (me->action() == tgt::MouseEvent::PRESSED || (!_mousePressedInRaycaster && me->action() == tgt::MouseEvent::WHEEL)) {
                if (me->x() <= p_smallRenderSize.getValue().x) {
                    if (me->y() <= p_smallRenderSize.getValue().y) {
                        _eventPositionOffset = tgt::ivec2(0, 0);
                        _eventViewportSize = p_smallRenderSize.getValue();
                        _viewUnderEvent = (p_largeView.getOptionValue() == XY_PLANE) ? VOLUME : XY_PLANE;
                    }
                    else if (me->y() <= 2*p_smallRenderSize.getValue().y) {
                        _eventPositionOffset = tgt::ivec2(0, -p_smallRenderSize.getValue().y);
                        _eventViewportSize = p_smallRenderSize.getValue();
                        _viewUnderEvent = (p_largeView.getOptionValue() == XZ_PLANE) ? VOLUME : XZ_PLANE;
                    }
                    else {
                        _eventPositionOffset = tgt::ivec2(0, -2 * p_smallRenderSize.getValue().y);
                        _eventViewportSize = p_smallRenderSize.getValue();
                        _viewUnderEvent = (p_largeView.getOptionValue() == YZ_PLANE) ? VOLUME : YZ_PLANE;
                    }
                }
                else {
                    _eventPositionOffset = tgt::ivec2(- p_smallRenderSize.getValue().x, 0);
                    _eventViewportSize = p_largeRenderSize.getValue();
                    _viewUnderEvent = p_largeView.getOptionValue();
                }
            }

            // create a new MouseEvent for the corresponding subview
            tgt::MouseEvent adjustedMe(me->x() + _eventPositionOffset.x, me->y() + _eventPositionOffset.y,
                me->action(), me->modifiers(), me->button(),
                _eventViewportSize);

            // now divert the new MouseEvent to the corresponding handler
            if (me->action() == tgt::MouseEvent::DOUBLECLICK) {
                p_largeView.selectByOption(_viewUnderEvent);
            }
            else if (_mousePressedInRaycaster || _viewUnderEvent == VOLUME) {
                // raycasting trackball navigation
                if (me->action() == tgt::MouseEvent::PRESSED)
                    _mousePressedInRaycaster = true;
                else if (me->action() == tgt::MouseEvent::RELEASED)
                    _mousePressedInRaycaster = false;
                _trackballEH->onEvent(&adjustedMe);
            }
            else if (me->action() == tgt::MouseEvent::WHEEL) {
                // Mouse wheel has changed -> cycle slices
                if (_viewUnderEvent == XY_PLANE) {
                    _zSliceHandler.onEvent(e);
                }
                else if (_viewUnderEvent == XZ_PLANE) {
                    _ySliceHandler.onEvent(e);
                }
                else {
                    _xSliceHandler.onEvent(e);
                }
            }
            else if (p_enableScribbling.getValue() && (me->modifiers() & tgt::Event::CTRL || me->modifiers() & tgt::Event::ALT)) {
                // CTRL pressed -> forward to SliceExtractor's scribbling
                if (me->action() == tgt::MouseEvent::PRESSED) {
                    _scribblePointer = (me->modifiers() & tgt::Event::CTRL) ? &_yesScribbles : &_noScribbles;
                    if (! (me->modifiers() & tgt::Event::SHIFT))
                        _scribblePointer->clear();
                }
                else if (_scribblePointer != nullptr && me->action() == tgt::MouseEvent::RELEASED) {
                    _scribblePointer = nullptr;
                }

                // lock this processor, so that the slice orientation's setting does not change
                AbstractProcessor::ScopedLock lock(this);

                if (_viewUnderEvent == XY_PLANE) {
                    _sliceRenderer->p_sliceOrientation.selectByOption(SliceExtractor::XY_PLANE);
                }
                else if (_viewUnderEvent == XZ_PLANE) {
                    _sliceRenderer->p_sliceOrientation.selectByOption(SliceExtractor::XZ_PLANE);
                }
                else {
                    _sliceRenderer->p_sliceOrientation.selectByOption(SliceExtractor::YZ_PLANE);
                }
                _sliceRenderer->onEvent(&adjustedMe);
            }
            else {
                // adjust slice TF windowing
                _windowingHandler.onEvent(&adjustedMe);
            }
        }
    }

    void VolumeExplorer::onSliceExtractorScribblePainted(tgt::vec3 voxel) {
        if (_scribblePointer != nullptr) {
            _scribblePointer->push_back(voxel);
            invalidate(INVALID_RESULT | SCRIBBLE_INVALID);
        }
    }

    VolumeRenderer* VolumeExplorer::getVolumeRenderer() {
        return &_raycaster;
    }

    SliceRenderProcessor* VolumeExplorer::getSliceRenderer() {
        return _sliceRenderer;
    }

}
