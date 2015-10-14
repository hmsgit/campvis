// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#include "cgt/assert.h"
#include "cgt/event/mouseevent.h"

#include "trackballcameraprovider.h"
#include "core/datastructures/cameradata.h"
#include "core/datastructures/imagedata.h"
#include "core/pipeline/visualizationprocessor.h"

namespace campvis {


    // = TrackballNavigationEventHandler ==============================================================
    
    static const GenericOption<TrackballCameraProvider::AutomationMode> automationOptions[3] = {
        GenericOption<TrackballCameraProvider::AutomationMode>("manual", "Fully Manual", TrackballCameraProvider::FullManual),
        GenericOption<TrackballCameraProvider::AutomationMode>("semiAutomatic", "SemiAutomatic", TrackballCameraProvider::SemiAutomatic),
        GenericOption<TrackballCameraProvider::AutomationMode>("automatic", "Fully Automatic", TrackballCameraProvider::FullAutomatic)
    };

    const std::string TrackballCameraProvider::loggerCat_ = "CAMPVis.modules.io.TrackballCameraProvider";

    TrackballCameraProvider::TrackballCameraProvider(IVec2Property* canvasSize) 
        : CameraProvider()
        , p_automationMode("AutomationMode", "Automation Mode", automationOptions, 3)
        , p_image("ReferenceImage", "Reference Image", "", DataNameProperty::READ)
        , p_llf("LLF", "Bounding Box LLF", cgt::vec3(0.f), cgt::vec3(-10000.f), cgt::vec3(10000.f))
        , p_urb("URB", "Bounding Box URB", cgt::vec3(1.f), cgt::vec3(-10000.f), cgt::vec3(10000.f))
        , _canvasSize(canvasSize)
        , _trackball(nullptr)
    {
        _dirty = false;
        p_automationMode.selectByOption(FullAutomatic);

        addProperty(p_automationMode, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_image, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_llf, INVALID_RESULT | INVALID_PROPERTIES);
        addProperty(p_urb, INVALID_RESULT | INVALID_PROPERTIES);

        if (_canvasSize != nullptr) {
            _canvasSize->s_changed.connect(this, &TrackballCameraProvider::onRenderTargetSizeChanged);
            _trackball = new cgt::Trackball(this, _canvasSize->getValue());
        }
        else {
            _trackball = new cgt::Trackball(this, cgt::ivec2(100, 100));
        }
    }

    TrackballCameraProvider::~TrackballCameraProvider() {

    }

    void TrackballCameraProvider::init() {
        onRenderTargetSizeChanged(_canvasSize);
    }

    void TrackballCameraProvider::deinit() {
        if (_canvasSize != nullptr)
            _canvasSize->s_changed.disconnect(this);

        delete _trackball;
    }

    void TrackballCameraProvider::onEvent(cgt::Event* e) {
        if (typeid(*e) == typeid(cgt::MouseEvent)) {
            cgt::MouseEvent* me = static_cast<cgt::MouseEvent*>(e);
            if (me->action() == cgt::MouseEvent::PRESSED) {
                for (std::vector<VisualizationProcessor*>::iterator it = _lqModeProcessors.begin(); it != _lqModeProcessors.end(); ++it)
                    (*it)->p_lqMode.setValue(true);
                _trackball->mousePressEvent(me);
            }
            else if (me->action() == cgt::MouseEvent::RELEASED) {
                for (std::vector<VisualizationProcessor*>::iterator it = _lqModeProcessors.begin(); it != _lqModeProcessors.end(); ++it)
                    (*it)->p_lqMode.setValue(false);
                _trackball->mouseReleaseEvent(me);
            }
            else if (me->action() == cgt::MouseEvent::MOTION) {
                _trackball->mouseMoveEvent(me);
            }
            else if (me->action() == cgt::MouseEvent::WHEEL)
                _trackball->wheelEvent(me);
        }
        else if (typeid(*e) == typeid(cgt::KeyEvent)) {
            _trackball->keyEvent(static_cast<cgt::KeyEvent*>(e));
        }
    }

    cgt::Camera* TrackballCameraProvider::getCamera() {
        if (!_dirty) {
            _localCopy = cgt::Camera(
                p_position.getValue(),
                p_focus.getValue(),
                p_upVector.getValue(),
                p_fov.getValue(),
                p_aspectRatio.getValue(),
                p_clippingPlanes.getValue().x,
                p_clippingPlanes.getValue().y,
                p_projectionMode.getOptionValue());
        }
        _dirty = true;
        return &_localCopy;
    }

    void TrackballCameraProvider::update() {
        ignorePropertyChanges();

        p_position.setValue(_localCopy.getPosition());
        p_focus.setValue(_localCopy.getFocus());
        p_upVector.setValue(_localCopy.getUpVector());
        p_fov.setValue(_localCopy.getFovy());
        p_aspectRatio.setValue(_localCopy.getRatio());
        if (cgt::isNumber(_localCopy.getNearDist()) && cgt::isNumber(_localCopy.getFarDist()))
            p_clippingPlanes.setValue(cgt::vec2(_localCopy.getNearDist(), _localCopy.getFarDist()));
        
        switch (_localCopy.getProjectionMode()) {
            case cgt::Camera::PERSPECTIVE:
                p_projectionMode.selectById("perspective");
                break;
            case cgt::Camera::ORTHOGRAPHIC:
                p_projectionMode.selectById("orthographic");
                break;
            case cgt::Camera::FRUSTUM:
                p_projectionMode.selectById("frustum");
                break;
        }

        _dirty = false;

        observePropertyChanges();
        invalidate(AbstractProcessor::INVALID_RESULT);
    }

    void TrackballCameraProvider::onRenderTargetSizeChanged(const AbstractProperty* prop) {
        float ratio = static_cast<float>(_canvasSize->getValue().x) / static_cast<float>(_canvasSize->getValue().y);
        _trackball->setViewprtSize(_canvasSize->getValue());
        _trackball->setWindowRatio(ratio);
        invalidate(INVALID_RESULT);
    }

    void TrackballCameraProvider::updateProperties(DataContainer& data) {
        if (p_automationMode.getOptionValue() == FullAutomatic) {
            // convert data
            ScopedTypedData<IHasWorldBounds> img(data, p_image.getValue());
            if (img != 0) {
                cgt::Bounds b = img->getWorldBounds();
                p_llf.setValue(b.getLLF());
                p_urb.setValue(b.getURB());

                reinitializeCamera(b);
            }

            p_position.setVisible(false);
            p_focus.setVisible(false);
            p_upVector.setVisible(false);
            p_fov.setVisible(false);
            p_aspectRatio.setVisible(false);
            p_clippingPlanes.setVisible(false);

            p_image.setVisible(true);
            p_llf.setVisible(false);
            p_urb.setVisible(false);
        }
        else if (p_automationMode.getOptionValue() == SemiAutomatic) {
            reinitializeCamera(cgt::Bounds(p_llf.getValue(), p_urb.getValue()));

            p_position.setVisible(false);
            p_focus.setVisible(false);
            p_upVector.setVisible(false);
            p_fov.setVisible(false);
            p_aspectRatio.setVisible(false);
            p_clippingPlanes.setVisible(false);

            p_image.setVisible(false);
            p_llf.setVisible(true);
            p_urb.setVisible(true);
        }
        else if (p_automationMode.getOptionValue() == FullManual) {
            p_position.setVisible(true);
            p_focus.setVisible(true);
            p_upVector.setVisible(true);
            p_fov.setVisible(true);
            p_aspectRatio.setVisible(true);
            p_clippingPlanes.setVisible(true);

            p_image.setVisible(false);
            p_llf.setVisible(false);
            p_urb.setVisible(false);
        }
    }

    void TrackballCameraProvider::addLqModeProcessor(VisualizationProcessor* vp) {
        cgtAssert(vp != 0, "Pointer to processor must not be 0.");
        cgtAssert(std::find(_lqModeProcessors.begin(), _lqModeProcessors.end(), vp) == _lqModeProcessors.end(), "Processor already in list of LQ mode processors.");

        _lqModeProcessors.push_back(vp);
    }

    void TrackballCameraProvider::removeLqModeProcessor(VisualizationProcessor* vp) {
        for (std::vector<VisualizationProcessor*>::iterator it = _lqModeProcessors.begin(); it != _lqModeProcessors.end(); ++it) {
            if (*it == vp) {
                _lqModeProcessors.erase(it);
                return;
            }
        }
    }

    void TrackballCameraProvider::setViewportSizeProperty(IVec2Property* viewportSizeProp) {
        cgtAssert(viewportSizeProp != nullptr, "Pointer must not be 0.");

        if (_canvasSize != nullptr) {
            _canvasSize->s_changed.disconnect(this);
        }

        _canvasSize = viewportSizeProp;
        _canvasSize->s_changed.connect(this, &TrackballCameraProvider::onRenderTargetSizeChanged);
        setPropertyInvalidationLevel(*viewportSizeProp, INVALID_RESULT);
        onRenderTargetSizeChanged(viewportSizeProp);
    }

    void TrackballCameraProvider::reinitializeCamera(const cgt::Bounds& worldBounds) {
        if (_trackball->getSceneBounds() != worldBounds) {
            cgt::vec3 pos = worldBounds.center() - cgt::vec3(0, 0, cgt::length(worldBounds.diagonal()));

            _trackball->setSceneBounds(worldBounds);
            _trackball->setCenter(worldBounds.center());
            _trackball->reinitializeCamera(pos, worldBounds.center(), p_upVector.getValue());
        }

    }

}