// ================================================================================================
// 
// This file is part of the KissCL, an OpenCL C++ wrapper following the KISS principle.
// 
// Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (software@cszb.net)
// 
// This library is free software; you can redistribute it and/or modify it under the terms of the 
// GNU Lesser General Public License version 3 as published by the Free Software Foundation.
// 
// This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See 
// the GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License in the file 
// "LICENSE.txt" along with this library. If not, see <http://www.gnu.org/licenses/>.
// 
// ================================================================================================

#ifndef KISSCL_EVENT_H__
#define KISSCL_EVENT_H__

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

#endif // KISSCL_EVENT_H__
