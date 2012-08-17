#ifndef CLEVENT_H__
#define CLEVENT_H__

#include "kisscl/kisscl.h"



namespace kisscl {

    /**
     * Wrapper for an OpenCL event.
     */
    class Event : public CLWrapper<cl_event> {
    public:
        /**
         * Creates a new wraper object for the given event.
         * \param   id  OpenCL event id.
         */
        Event(cl_event id);

    };

// = EventList starts here ========================================================================

    struct EventList {
        EventList();
        EventList(const Event& e1);
        EventList(const Event& e1, const Event& e2);
        ~EventList();

        cl_event* _events;
        size_t _size;
    };


}

#endif // CLEVENT_H__
