#include "commandqueue.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "cllib/context.h"
#include "cllib/device.h"
#include "cllib/event.h"
#include "cllib/kernel.h"
#include "cllib/memory.h"
#include "cllib/platform.h"

namespace cllib {

    const std::string CommandQueue::loggerCat_ = "cllib.CommandQueue";

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
        if (_id != 0)
            clReleaseCommandQueue(_id);
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

    Event CommandQueue::enqueueTask(const Kernel* kernel, const EventList& eventsToWaitFor /*= EventList()*/) {
        tgtAssert(kernel != 0, "Kernel must not be 0.");

        cl_event e;
        LCL_ERROR(clEnqueueTask(_id, kernel->getId(), eventsToWaitFor._size, eventsToWaitFor._events, &e));
        return Event(e);
    }

    Event CommandQueue::enqueueKernel(const Kernel* kernel, size_t globalWorkSize, size_t localWorkSize /*= 0*/, size_t offset /*= 0*/, const EventList& eventsToWaitFor /*= EventList()*/) {
        tgtAssert(kernel != 0, "Kernel must not be 0.");
        tgtAssert(localWorkSize != 0 && localWorkSize > globalWorkSize, "Global work size must be greater than local work size.");
        tgtAssert(localWorkSize != 0 && (globalWorkSize % localWorkSize != 0), "Global work size must be a multiple than local work size.");

        cl_event e;
        LCL_ERROR(clEnqueueNDRangeKernel(
            _id, 
            kernel->getId(), 
            1, 
            (offset == 0 ? 0 : &offset), 
            &globalWorkSize, 
            (localWorkSize == 0 ? 0 : &localWorkSize), 
            eventsToWaitFor._size, 
            eventsToWaitFor._events, 
            &e));
        return Event(e);
    }

    Event CommandQueue::enqueueKernel(const Kernel* kernel, tgt::svec2 globalWorkSize, tgt::svec2 localWorkSize /*= tgt::svec2::zero*/, tgt::svec2 offset /*= tgt::svec2::zero*/, const EventList& eventsToWaitFor /*= EventList()*/) {
        tgtAssert(kernel != 0, "Kernel must not be 0.");
        tgtAssert(localWorkSize != tgt::svec2::zero && tgt::hor(tgt::greaterThan(localWorkSize, globalWorkSize)), "Global work size must be greater than local work size.");
        tgtAssert(localWorkSize != tgt::svec2::zero && (globalWorkSize.x % localWorkSize.x != 0), "Global work size must be a multiple than local work size.");
        tgtAssert(localWorkSize != tgt::svec2::zero && (globalWorkSize.y % localWorkSize.y != 0), "Global work size must be a multiple than local work size.");

        cl_event e;
        LCL_ERROR(clEnqueueNDRangeKernel(
            _id, 
            kernel->getId(), 
            2, 
            (offset == tgt::svec2::zero ? 0 : offset.elem), 
            globalWorkSize.elem,
            (localWorkSize == tgt::svec2::zero ? 0 : localWorkSize.elem),
            eventsToWaitFor._size, 
            eventsToWaitFor._events, 
            &e));
        return Event(e);
    }

    Event CommandQueue::enqueueKernel(const Kernel* kernel, tgt::svec3 globalWorkSize, tgt::svec3 localWorkSize /*= tgt::svec3::zero*/, tgt::svec3 offset /*= tgt::svec3::zero*/, const EventList& eventsToWaitFor /*= EventList()*/) {
        tgtAssert(kernel != 0, "Kernel must not be 0.");
        tgtAssert(localWorkSize != tgt::svec3::zero && tgt::hor(tgt::greaterThan(localWorkSize, globalWorkSize)), "Global work size must be greater than local work size.");
        tgtAssert(localWorkSize != tgt::svec3::zero && (globalWorkSize.x % localWorkSize.x != 0), "Global work size must be a multiple than local work size.");
        tgtAssert(localWorkSize != tgt::svec3::zero && (globalWorkSize.y % localWorkSize.y != 0), "Global work size must be a multiple than local work size.");
        tgtAssert(localWorkSize != tgt::svec3::zero && (globalWorkSize.z % localWorkSize.z != 0), "Global work size must be a multiple than local work size.");

        cl_event e;
        LCL_ERROR(clEnqueueNDRangeKernel(
            _id, 
            kernel->getId(), 
            3, 
            (offset == tgt::svec3::zero ? 0 : offset.elem), 
            globalWorkSize.elem,
            (localWorkSize == tgt::svec3::zero ? 0 : localWorkSize.elem),
            eventsToWaitFor._size, 
            eventsToWaitFor._events, 
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

    void CommandQueue::enqueueWaitForEvents(const EventList& eventsToWaitFor /*= EventList()*/) {
        LCL_ERROR(clEnqueueWaitForEvents(_id, eventsToWaitFor._size, eventsToWaitFor._events));
    }

    Event CommandQueue::enqueueRead(const Buffer* buffer, void* data, bool blocking /*= true*/, size_t offset /*= 0*/, size_t numBytes /*= 0*/, const EventList& eventsToWaitFor /*= EventList()*/) {
        cl_event e;
        if (numBytes == 0) {
            LCL_ERROR(clEnqueueReadBuffer(_id, buffer->getId(), blocking, offset, buffer->getSize(), data, eventsToWaitFor._size, eventsToWaitFor._events, &e));
        }
        else {
            LCL_ERROR(clEnqueueReadBuffer(_id, buffer->getId(), blocking, offset, numBytes, data, eventsToWaitFor._size, eventsToWaitFor._events, &e));
        }
        return Event(e);
    }

    Event CommandQueue::enqueueWrite(const Buffer* buffer, void* data, bool blocking /*= true*/, size_t offset /*= 0*/, size_t numBytes /*= 0*/, const EventList& eventsToWaitFor /*= EventList()*/) {
        cl_event e;
        if (numBytes == 0) {
            LCL_ERROR(clEnqueueWriteBuffer(_id, buffer->getId(), blocking, offset, buffer->getSize(), data, eventsToWaitFor._size, eventsToWaitFor._events, &e));
        }
        else {
            LCL_ERROR(clEnqueueWriteBuffer(_id, buffer->getId(), blocking, offset, numBytes, data, eventsToWaitFor._size, eventsToWaitFor._events, &e));
        }
        return Event(e);
    }



}
