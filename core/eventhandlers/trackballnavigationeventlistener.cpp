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

#include "trackballnavigationeventlistener.h"
#include "cgt/assert.h"
#include "cgt/event/mouseevent.h"
#include "core/datastructures/abstractdata.h"
#include "core/properties/cameraproperty.h"
#include "core/pipeline/visualizationprocessor.h"

namespace campvis {

    CamPropNavigationWrapper::CamPropNavigationWrapper(CameraProperty* camProp)
        : _cameraProperty(camProp)
        , _dirty(false)
    {

    }

    CamPropNavigationWrapper::~CamPropNavigationWrapper() {

    }

    cgt::Camera* CamPropNavigationWrapper::getCamera() {
        if (! _dirty) {
            // perform deep copy (hopefully thread-safe...)
            _localCopy = _cameraProperty->getValue();
            _dirty = true;
        }
        return &_localCopy;
    }

    void CamPropNavigationWrapper::update() {
        _cameraProperty->setValue(_localCopy);
        _dirty = false;
    }

// = TrackballNavigationEventListener ==============================================================

    const std::string TrackballNavigationEventListener::loggerCat_ = "CAMPVis.core.eventhandler.TrackballNavigationEventListener";

    TrackballNavigationEventListener::TrackballNavigationEventListener(CameraProperty* cameraProperty, IVec2Property* viewportSizeProp)
        : cgt::EventListener()
        , _cameraProperty(cameraProperty)
        , _viewportSizeProp(viewportSizeProp)
        , _cpnw(cameraProperty)
        , _trackball(0)
        , _sceneBounds(cgt::vec3(0.f))
    {
        cgtAssert(_cameraProperty != 0, "Assigned camera property must not be 0.");
        cgtAssert(_viewportSizeProp != 0, "Assigned viewport size property must not be 0.");

        _trackball = new cgt::Trackball(&_cpnw, viewportSizeProp->getValue());
        _viewportSizeProp->s_changed.connect(this, &TrackballNavigationEventListener::onViewportSizePropChanged);
    }

    TrackballNavigationEventListener::~TrackballNavigationEventListener() {
        _viewportSizeProp->s_changed.disconnect(this);
        delete _trackball;
    }

    void TrackballNavigationEventListener::onEvent(cgt::Event* e) {
        if (typeid(*e) == typeid(cgt::MouseEvent)) {
            cgt::MouseEvent* me = static_cast<cgt::MouseEvent*>(e);
            _trackball->setViewprtSize(me->viewport());
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
            else if (me->action() == cgt::MouseEvent::MOTION)
                _trackball->mouseMoveEvent(me);
            else if (me->action() == cgt::MouseEvent::WHEEL)
                _trackball->wheelEvent(me);
        }
        else if (typeid(*e) == typeid(cgt::KeyEvent)) {
            _trackball->keyEvent(static_cast<cgt::KeyEvent*>(e));
        }
    }

    void TrackballNavigationEventListener::reinitializeCamera(const cgt::vec3& position, const cgt::vec3& focus, const cgt::vec3& upVector) {
        _trackball->reinitializeCamera(position, focus, upVector);
    }

    void TrackballNavigationEventListener::reinitializeCamera(const IHasWorldBounds* hwb) {
        reinitializeCamera(hwb->getWorldBounds());
    }

    void TrackballNavigationEventListener::reinitializeCamera(const cgt::Bounds& worldBounds) {
        if (_sceneBounds != worldBounds) {
            cgt::vec3 pos = worldBounds.center() - cgt::vec3(0, 0, cgt::length(worldBounds.diagonal()));

            setSceneBounds(worldBounds);
            setCenter(worldBounds.center());
            reinitializeCamera(pos, worldBounds.center(), _cameraProperty->getValue().getUpVector());
        }
    }
    
    void TrackballNavigationEventListener::setCenter(const cgt::vec3& center) {
        _trackball->setCenter(center);
    }
    
    void TrackballNavigationEventListener::setSceneBounds(const cgt::Bounds& bounds) {
        _sceneBounds = bounds;
        _trackball->setSceneBounds(bounds);
    }

    const cgt::Bounds& TrackballNavigationEventListener::getSceneBounds() const {
        return _trackball->getSceneBounds();
    }

    cgt::Trackball* TrackballNavigationEventListener::getTrackball() {
        return _trackball;
    }

    void TrackballNavigationEventListener::addLqModeProcessor(VisualizationProcessor* vp) {
        cgtAssert(vp != 0, "Pointer to processor must not be 0.");
        cgtAssert(std::find(_lqModeProcessors.begin(), _lqModeProcessors.end(), vp) == _lqModeProcessors.end(), "Processor already in list of LQ mode processors.");

        _lqModeProcessors.push_back(vp);
    }

    void TrackballNavigationEventListener::removeLqModeProcessor(VisualizationProcessor* vp) {
        for (std::vector<VisualizationProcessor*>::iterator it = _lqModeProcessors.begin(); it != _lqModeProcessors.end(); ++it) {
            if (*it == vp) {
                _lqModeProcessors.erase(it);
                return;
            }
        }
    }

    void TrackballNavigationEventListener::onViewportSizePropChanged(const AbstractProperty* p) {
        cgtAssert(p == _viewportSizeProp, "Signal from unexpected property! Expected p == _viewportSizeProp.");

        _trackball->setViewprtSize(_viewportSizeProp->getValue());
        float ratio = static_cast<float>(_viewportSizeProp->getValue().x) / static_cast<float>(_viewportSizeProp->getValue().y);

        if (ratio == 0) {
            LERROR("Window ratio must not be 0.");
        }
        else {
            _trackball->setWindowRatio(ratio);
        }
    }

    void TrackballNavigationEventListener::setViewportSizeProperty(IVec2Property* viewportSizeProp) {
        _viewportSizeProp->s_changed.disconnect(this);

        cgtAssert(viewportSizeProp != nullptr, "The property must not be 0.");
        _viewportSizeProp = viewportSizeProp;
        onViewportSizePropChanged(_viewportSizeProp);
        _viewportSizeProp->s_changed.connect(this, &TrackballNavigationEventListener::onViewportSizePropChanged);
    }

}
