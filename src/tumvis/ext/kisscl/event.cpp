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

#include "event.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "kisscl/device.h"
#include "kisscl/platform.h"

namespace kisscl {

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