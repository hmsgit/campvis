#include "event.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "cllib/device.h"
#include "cllib/platform.h"

namespace cllib {

    Event::Event(cl_event id)
        : CLWrapper<cl_event>(id)
    {
        tgtAssert(_id != 0, "Event ID must not be 0.");
    }


    EventList::EventList() {
        _events = 0;
        _size = 0;
    }

    EventList::EventList(const Event& e1) {
        _size = 1;
        _events = new cl_event[_size];
        _events[0] = e1.getId();
    }

    EventList::EventList(const Event& e1, const Event& e2) {
        _size = 2;
        _events = new cl_event[_size];
        _events[0] = e1.getId();
        _events[1] = e2.getId();
    }


    EventList::~EventList() {

    }

}
