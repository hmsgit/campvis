#include "mwheeltonumericpropertyeventhandler.h"
#include "tgt/assert.h"
#include "tgt/event/mouseevent.h"

namespace TUMVis {
    const std::string MWheelToNumericPropertyEventHandler::loggerCat_ = "TUMVis.core.eventhandler.MWheelToNumericPropertyEventHandler";

    MWheelToNumericPropertyEventHandler::MWheelToNumericPropertyEventHandler(INumericProperty* property)
        : AbstractEventHandler()
        , _prop(property)
    {
        tgtAssert(_prop != 0, "Assigned property must not be 0.");
    }

    MWheelToNumericPropertyEventHandler::~MWheelToNumericPropertyEventHandler() {

    }

    bool MWheelToNumericPropertyEventHandler::accept(tgt::Event* e) {
        if (tgt::MouseEvent* me = dynamic_cast<tgt::MouseEvent*>(e)) {
            return me->action() == tgt::MouseEvent::WHEEL;
        }
        return false;
    }

    void MWheelToNumericPropertyEventHandler::execute(tgt::Event* e) {
        tgtAssert(dynamic_cast<tgt::MouseEvent*>(e) != 0, "Given event has wrong type. Check if the event is accepted by this event handler before executing it!");

        // this is only to be executed when the event was accepted, so the static cast is safe.
        tgt::MouseEvent* me = static_cast<tgt::MouseEvent*>(e);
        tgtAssert(me->action() == tgt::MouseEvent::WHEEL, "Given event has wrong type. Check if the event is accepted by this event handler before executing it!");
        switch (me->button()) {
            case tgt::MouseEvent::MOUSE_WHEEL_UP:
                _prop->increment();
                e->ignore();
                break;
            case tgt::MouseEvent::MOUSE_WHEEL_DOWN:
                _prop->decrement();
                e->ignore();
                break;
        }
    }

}
