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

#include "trackballnavigationeventlistener.h"
#include "tgt/assert.h"
#include "tgt/event/mouseevent.h"
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

    tgt::Camera* CamPropNavigationWrapper::getCamera() {
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

    TrackballNavigationEventListener::TrackballNavigationEventListener(CameraProperty* cameraProperty, const tgt::ivec2& viewportSize)
        : tgt::EventListener()
        , _cameraProperty(cameraProperty)
        , _cpnw(cameraProperty)
        , _trackball(0)
    {
        tgtAssert(_cameraProperty != 0, "Assigned property must not be 0.");
        _trackball = new tgt::Trackball(&_cpnw, viewportSize);
    }

    TrackballNavigationEventListener::~TrackballNavigationEventListener() {
        delete _trackball;
    }

    void TrackballNavigationEventListener::onEvent(tgt::Event* e) {
        if (typeid(*e) == typeid(tgt::MouseEvent)) {
            tgt::MouseEvent* me = static_cast<tgt::MouseEvent*>(e);
            if (me->action() == tgt::MouseEvent::PRESSED) {
                for (std::vector<VisualizationProcessor*>::iterator it = _lqModeProcessors.begin(); it != _lqModeProcessors.end(); ++it)
                    (*it)->p_lqMode.setValue(true);
                _trackball->mousePressEvent(me);
            }
            else if (me->action() == tgt::MouseEvent::RELEASED) {
                for (std::vector<VisualizationProcessor*>::iterator it = _lqModeProcessors.begin(); it != _lqModeProcessors.end(); ++it)
                    (*it)->p_lqMode.setValue(false);
                _trackball->mouseReleaseEvent(me);
            }
            else if (me->action() == tgt::MouseEvent::MOTION)
                _trackball->mouseMoveEvent(me);
            else if (me->action() == tgt::MouseEvent::WHEEL)
                _trackball->wheelEvent(me);
        }
        else if (typeid(*e) == typeid(tgt::KeyEvent)) {
            _trackball->keyEvent(static_cast<tgt::KeyEvent*>(e));
        }
    }

    void TrackballNavigationEventListener::reinitializeCamera(const tgt::vec3& position, const tgt::vec3& focus, const tgt::vec3& upVector) {
        _trackball->reinitializeCamera(position, focus, upVector);
    }

    void TrackballNavigationEventListener::setCenter(const tgt::vec3& center) {
        _trackball->setCenter(center);
    }

    void TrackballNavigationEventListener::setViewportSize(const tgt::ivec2& viewportSize) {
        _trackball->setViewprtSize(viewportSize);
    }

    void TrackballNavigationEventListener::setSceneBounds(const tgt::Bounds& bounds) {
        _trackball->setSceneBounds(bounds);
    }

    const tgt::Bounds& TrackballNavigationEventListener::getSceneBounds() const {
        return _trackball->getSceneBounds();
    }

    void TrackballNavigationEventListener::addLqModeProcessor(VisualizationProcessor* vp) {
        tgtAssert(vp != 0, "Pointer to processor must not be 0.");
        tgtAssert(std::find(_lqModeProcessors.begin(), _lqModeProcessors.end(), vp) == _lqModeProcessors.end(), "Processor already in list of LQ mode processors.");

        _lqModeProcessors.push_back(vp);
    }

    void TrackballNavigationEventListener::removeLqModeProcessor(VisualizationProcessor* vp) {
        for (std::vector<VisualizationProcessor*>::iterator it = _lqModeProcessors.begin(); it != _lqModeProcessors.end(); ++it) {
            if (*it == vp)
                _lqModeProcessors.erase(it);
        }
    }

}
