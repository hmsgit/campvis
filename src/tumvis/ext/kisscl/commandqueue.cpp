// ================================================================================================
// 
// This file is part of the KissCL, an OpenCL C++ wrapper following the KISS principle.
// 
// Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (software@cszb.net)
// 
// Redistribution and use in source and binary forms, with or without modification, are permitted
// provided that the following conditions are met:
// 
//  * Redistributions of source code must retain the above copyright notice, this list of
//    conditions and the following disclaimer.
//  * Redistributions in binary form must reproduce the above copyright notice, this list of
//    conditions and the following disclaimer in the documentation and/or other materials provided
//    with the distribution.
//  * Neither the name of the <ORGANIZATION> nor the names of its contributors may be used to
//    endorse or promote products derived from this software without specific prior written
//    permission.
//  
//  THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
//  IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY
//  AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR 
//  CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
//  CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
//  SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY
//  THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR
//  OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
//  POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#include "commandqueue.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "kisscl/context.h"
#include "kisscl/device.h"
#include "kisscl/event.h"
#include "kisscl/kernel.h"
#include "kisscl/memory.h"
#include "kisscl/platform.h"

namespace kisscl {

    const std::string CommandQueue::loggerCat_ = "kisscl.CommandQueue";

    CommandQueue::CommandQueue(Context* context, cl_command_queue_properties properties /*= 0*/)
        : CLWrapper<cl_command_queue>(0)
        , _context(context)
        , _device(context->getDevices().front())
        , _profilingEnabled((properties & CL_QUEUE_PROFILING_ENABLE) != 0)
    {
        tgtAssert(_context != 0, "Context must not be 0.");
        tgtAssert(_device != 0, "Device must not be 0. Something went terribly wrong, this should should have been asserted earlier.");

        cl_int err;
        _id = clCreateCommandQueue(_context->getId(), _device->getId(), properties, &err);
        LCL_ERROR(err);
    }

    CommandQueue::CommandQueue(Context* context, Device* device, cl_command_queue_properties properties /*= 0*/)
        : CLWrapper<cl_command_queue>(0)
        , _context(context)
        , _device(device)
        , _profilingEnabled((properties & CL_QUEUE_PROFILING_ENABLE) != 0)
    {
        tgtAssert(_context != 0, "Context must not be 0.");
        tgtAssert(_device != 0, "Device must not be 0.");

        cl_int err;
        _id = clCreateCommandQueue(_context->getId(), _device->getId(), properties, &err);
        LCL_ERROR(err);
    }

// = getters and setters ==========================================================================

    CommandQueue::~CommandQueue() {

    }

    const Context* CommandQueue::getContext() const {
        return _context;
    }

    const Device* CommandQueue::getDevice() const {
        return _device;
    }

// = the interesting stuff :) =====================================================================

    void CommandQueue::flush() {
        LCL_ERROR(clFlush(_id));
    }

    void CommandQueue::finish() {
        LCL_ERROR(clFinish(_id));
    }

    Event CommandQueue::enqueueTask(const Kernel* kernel, const ItemList<Event>& eventsToWaitFor /*= ItemList<Event>()*/) {
        tgtAssert(kernel != 0, "Kernel must not be 0.");

        cl_event e;
        LCL_ERROR(clEnqueueTask(_id, kernel->getId(), eventsToWaitFor._size, eventsToWaitFor._items, &e));
        return Event(e);
    }

    Event CommandQueue::enqueueKernel(const Kernel* kernel, size_t globalWorkSize, size_t localWorkSize /*= 0*/, size_t offset /*= 0*/, const ItemList<Event>& eventsToWaitFor /*= ItemList<Event>()*/) {
        tgtAssert(kernel != 0, "Kernel must not be 0.");
        tgtAssert(localWorkSize == 0 || localWorkSize < globalWorkSize, "Global work size must be greater than local work size.");
        tgtAssert(localWorkSize == 0 || (globalWorkSize % localWorkSize == 0), "Global work size must be a multiple than local work size.");

        cl_event e;
        LCL_ERROR(clEnqueueNDRangeKernel(
            _id, 
            kernel->getId(), 
            1, 
            (offset == 0 ? 0 : &offset), 
            &globalWorkSize, 
            (localWorkSize == 0 ? 0 : &localWorkSize), 
            eventsToWaitFor._size, 
            eventsToWaitFor._items, 
            &e));
        return Event(e);
    }

    Event CommandQueue::enqueueKernel(const Kernel* kernel, tgt::svec2 globalWorkSize, tgt::svec2 localWorkSize /*= tgt::svec2::zero*/, tgt::svec2 offset /*= tgt::svec2::zero*/, const ItemList<Event>& eventsToWaitFor /*= ItemList<Event>()*/) {
        tgtAssert(kernel != 0, "Kernel must not be 0.");
        tgtAssert(localWorkSize == tgt::svec2::zero || tgt::hor(tgt::lessThan(localWorkSize, globalWorkSize)), "Global work size must be greater than local work size.");
        tgtAssert(localWorkSize == tgt::svec2::zero || (globalWorkSize.x % localWorkSize.x == 0), "Global work size must be a multiple than local work size.");
        tgtAssert(localWorkSize == tgt::svec2::zero || (globalWorkSize.y % localWorkSize.y == 0), "Global work size must be a multiple than local work size.");

        cl_event e;
        LCL_ERROR(clEnqueueNDRangeKernel(
            _id, 
            kernel->getId(), 
            2, 
            (offset == tgt::svec2::zero ? 0 : offset.elem), 
            globalWorkSize.elem,
            (localWorkSize == tgt::svec2::zero ? 0 : localWorkSize.elem),
            eventsToWaitFor._size, 
            eventsToWaitFor._items, 
            &e));
        return Event(e);
    }

    Event CommandQueue::enqueueKernel(const Kernel* kernel, tgt::svec3 globalWorkSize, tgt::svec3 localWorkSize /*= tgt::svec3::zero*/, tgt::svec3 offset /*= tgt::svec3::zero*/, const ItemList<Event>& eventsToWaitFor /*= ItemList<Event>()*/) {
        tgtAssert(kernel != 0, "Kernel must not be 0.");
        tgtAssert(localWorkSize == tgt::svec3::zero || tgt::hor(tgt::lessThan (localWorkSize, globalWorkSize)), "Global work size must be greater than local work size.");
        tgtAssert(localWorkSize == tgt::svec3::zero || (globalWorkSize.x % localWorkSize.x == 0), "Global work size must be a multiple than local work size.");
        tgtAssert(localWorkSize == tgt::svec3::zero || (globalWorkSize.y % localWorkSize.y == 0), "Global work size must be a multiple than local work size.");
        tgtAssert(localWorkSize == tgt::svec3::zero || (globalWorkSize.z % localWorkSize.z == 0), "Global work size must be a multiple than local work size.");

        cl_event e;
        LCL_ERROR(clEnqueueNDRangeKernel(
            _id, 
            kernel->getId(), 
            3, 
            (offset == tgt::svec3::zero ? 0 : offset.elem), 
            globalWorkSize.elem,
            (localWorkSize == tgt::svec3::zero ? 0 : localWorkSize.elem),
            eventsToWaitFor._size, 
            eventsToWaitFor._items, 
            &e));
        return Event(e);
    }

    void CommandQueue::enqueueBarrier() {
        LCL_ERROR(clEnqueueBarrier(_id));
    }

    Event CommandQueue::enqueueMarker() {
        cl_event e;
        LCL_ERROR(clEnqueueMarker(_id, &e));
        return Event(e);
    }

    void CommandQueue::enqueueWaitForEvents(const ItemList<Event>& eventsToWaitFor /*= ItemList<Event>()*/) {
        LCL_ERROR(clEnqueueWaitForEvents(_id, eventsToWaitFor._size, eventsToWaitFor._items));
    }

    Event CommandQueue::enqueueRead(const Buffer* buffer, void* data, bool blocking /*= true*/, size_t offset /*= 0*/, size_t numBytes /*= 0*/, const ItemList<Event>& eventsToWaitFor /*= ItemList<Event>()*/) {
        cl_event e;
        if (numBytes == 0) {
            LCL_ERROR(clEnqueueReadBuffer(_id, buffer->getId(), blocking, offset, buffer->getSize(), data, eventsToWaitFor._size, eventsToWaitFor._items, &e));
        }
        else {
            LCL_ERROR(clEnqueueReadBuffer(_id, buffer->getId(), blocking, offset, numBytes, data, eventsToWaitFor._size, eventsToWaitFor._items, &e));
        }
        return Event(e);
    }

    Event CommandQueue::enqueueWrite(const Buffer* buffer, void* data, bool blocking /*= true*/, size_t offset /*= 0*/, size_t numBytes /*= 0*/, const ItemList<Event>& eventsToWaitFor /*= ItemList<Event>()*/) {
        cl_event e;
        if (numBytes == 0) {
            LCL_ERROR(clEnqueueWriteBuffer(_id, buffer->getId(), blocking, offset, buffer->getSize(), data, eventsToWaitFor._size, eventsToWaitFor._items, &e));
        }
        else {
            LCL_ERROR(clEnqueueWriteBuffer(_id, buffer->getId(), blocking, offset, numBytes, data, eventsToWaitFor._size, eventsToWaitFor._items, &e));
        }
        return Event(e);
    }

    Event CommandQueue::enqueueAcquireGLObject(const ItemList<GLTexture>& textures, const ItemList<Event>& eventsToWaitFor /*= ItemList<Event>()*/) {
        cl_event e;
        LCL_ERROR(clEnqueueAcquireGLObjects(_id, textures._size, textures._items, eventsToWaitFor._size, eventsToWaitFor._items, &e));
        return Event(e);
    }

    Event CommandQueue::enqueueReleaseGLObject(const ItemList<GLTexture>& textures, const ItemList<Event>& eventsToWaitFor /*= ItemList<Event>()*/) {
        cl_event e;
        LCL_ERROR(clEnqueueReleaseGLObjects(_id, textures._size, textures._items, eventsToWaitFor._size, eventsToWaitFor._items, &e));
        return Event(e);
    }



}
