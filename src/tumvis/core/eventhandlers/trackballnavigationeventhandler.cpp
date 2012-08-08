#include "trackballnavigationeventhandler.h"
#include "tgt/assert.h"
#include "tgt/event/mouseevent.h"
#include "core/properties/cameraproperty.h"

namespace TUMVis {

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

// = TrackballNavigationEventHandler ==============================================================

    const std::string TrackballNavigationEventHandler::loggerCat_ = "TUMVis.core.eventhandler.TrackballNavigationEventHandler";

    TrackballNavigationEventHandler::TrackballNavigationEventHandler(CameraProperty* cameraProperty, const tgt::ivec2& viewportSize)
        : AbstractEventHandler()
        , _cameraProperty(cameraProperty)
        , _cpnw(cameraProperty)
        , _trackball(0)
    {
        tgtAssert(_cameraProperty != 0, "Assigned property must not be 0.");
        _trackball = new tgt::Trackball(&_cpnw, viewportSize);
    }

    TrackballNavigationEventHandler::~TrackballNavigationEventHandler() {
        delete _trackball;
    }

    bool TrackballNavigationEventHandler::accept(tgt::Event* e) {
        if (typeid(*e) == typeid(tgt::MouseEvent)) {
            tgt::MouseEvent* me = static_cast<tgt::MouseEvent*>(e);
            if (me->action() == tgt::MouseEvent::PRESSED)
                return true;
            else if (me->action() == tgt::MouseEvent::RELEASED)
                return true;
            else if (me->action() == tgt::MouseEvent::MOTION)
                return true;
            else if (me->action() == tgt::MouseEvent::WHEEL)
                return true;
        }
        else if (typeid(*e) == typeid(tgt::KeyEvent)) {
            return true;
        }
        return false;
    }

    void TrackballNavigationEventHandler::execute(tgt::Event* e) {
        if (typeid(*e) == typeid(tgt::MouseEvent)) {
            tgt::MouseEvent* me = static_cast<tgt::MouseEvent*>(e);
            if (me->action() == tgt::MouseEvent::PRESSED)
                _trackball->mousePressEvent(me);
            else if (me->action() == tgt::MouseEvent::RELEASED)
                _trackball->mouseReleaseEvent(me);
            else if (me->action() == tgt::MouseEvent::MOTION)
                _trackball->mouseMoveEvent(me);
            else if (me->action() == tgt::MouseEvent::WHEEL)
                _trackball->wheelEvent(me);
        }
        else if (typeid(*e) == typeid(tgt::KeyEvent)) {
            _trackball->keyEvent(static_cast<tgt::KeyEvent*>(e));
        }
    }

    void TrackballNavigationEventHandler::reinitializeCamera(const tgt::vec3& position, const tgt::vec3& focus, const tgt::vec3& upVector) {
        _trackball->reinitializeCamera(position, focus, upVector);
    }

    void TrackballNavigationEventHandler::setCenter(const tgt::vec3& center) {
        _trackball->setCenter(center);
    }

}
