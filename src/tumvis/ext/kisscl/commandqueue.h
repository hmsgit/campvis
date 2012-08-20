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

#ifndef KISSCL_COMMANDQUEUE_H__
#define KISSCL_COMMANDQUEUE_H__

#include "tgt/vector.h"
#include "kisscl/kisscl.h"
#include "kisscl/event.h"

#include <vector>


namespace kisscl {
    class Buffer;
    class Context;
    class Device;
    class Kernel;
    class SharedTexture;
    
    /**
     * Wrapper class for an OpenCL command queue.
     * 
     * \todo    OpenCL command queues internally maintain a reference count. We probably should use it here => implement copy constructor/assignment op.
     */
    class CommandQueue : public CLWrapper<cl_command_queue> {
    public:
        /**
         * Creates a new command queue for the given OpenCL context and its first device.
         * \param   context     OpenCL context to create the command queue for. 
         * \param   properties  Command queue properties bitfield.
         */
        CommandQueue(Context* context, cl_command_queue_properties properties = 0);

        /**
         * Creates a new command queue for the given OpenCL context and the given OpenCL device.
         * \param   context     OpenCL context to create the command queue for. 
         * \param   device      OpenCL device to create the command queue for. 
         * \param   properties  Command queue properties bitfield.
         */
        CommandQueue(Context* context, Device* device, cl_command_queue_properties properties = 0);

        /**
         * Destructor, releases the OpenCL command queue.
         * \todo    Reference counting.
         */
        ~CommandQueue();

// = getters and setters ==========================================================================

        /**
         * Gets the OpenCL context for which the command queue is created.
         * \return _context
         */
        const Context* getContext() const;

        /**
         * Gets the OpenCL device for which the command queue is created.
         * \return _device
         */
        const Device* getDevice() const;

// = the interesting stuff :) =====================================================================

        /**
         * Issues a clFlush() on this command queue.
         */
        void flush();

        /**
         * Issues a clFinish() on this command queue.
         */
        void finish();


        /**
         * Enqueues an OpenCL task (Kernel executed as single work-item) to this command queue.
         * \param   kernel          The kernel to execute.
         * \param   eventsToWaitFor List of Events to wait for, default is empty.
         * \return  Event object that identifies this particular kernel execution instance.
         */
        Event enqueueTask(const Kernel* kernel, const EventList& eventsToWaitFor = EventList());

        /**
         * Enqueues an one-dimensional OpenCL kernel to this command queue.
         * \param   kernel          The kernel to execute.
         * \param   globalWorkSize  Global work size for the kernel (number of work-items).
         * \param   localWorkSize   Local work size (work group size), defaults 0 to let the OpenCL implementation choose (not recommended).
         * \param   offset          Offset to calculate the global ID of a work-item, default is 0.
         * \param   eventsToWaitFor List of Events to wait for, default is empty.
         * \return  Event object that identifies this particular kernel execution instance.
         */
        Event enqueueKernel(const Kernel* kernel, size_t globalWorkSize, size_t localWorkSize = 0, size_t offset = 0, const EventList& eventsToWaitFor = EventList());

        /**
         * Enqueues a two-dimensional OpenCL kernel to this command queue.
         * \param   kernel          The kernel to execute.
         * \param   globalWorkSize  Global work size for the kernel (number of work-items).
         * \param   localWorkSize   Local work size (work group size), defaults to (0, 0) to let the OpenCL implementation choose (not recommended).
         * \param   offset          Offset to calculate the global ID of a work-item, default is (0, 0).
         * \param   eventsToWaitFor List of Events to wait for, default is empty.
         * \return  Event object that identifies this particular kernel execution instance.
         */
        Event enqueueKernel(const Kernel* kernel, tgt::svec2 globalWorkSize, tgt::svec2 localWorkSize = tgt::svec2::zero, tgt::svec2 offset = tgt::svec2::zero, const EventList& eventsToWaitFor = EventList());

        /**
         * Enqueues a three-dimensional OpenCL kernel to this command queue.
         * \param   kernel          The kernel to execute.
         * \param   globalWorkSize  Global work size for the kernel (number of work-items).
         * \param   localWorkSize   Local work size (work group size), defaults to (0, 0, 0) to let the OpenCL implementation choose (not recommended).
         * \param   offset          Offset to calculate the global ID of a work-item, default is (0, 0, 0).
         * \param   eventsToWaitFor List of Events to wait for, default is empty.
         * \return  Event object that identifies this particular kernel execution instance.
         */
        Event enqueueKernel(const Kernel* kernel, tgt::svec3 globalWorkSize, tgt::svec3 localWorkSize = tgt::svec3::zero, tgt::svec3 offset = tgt::svec3::zero, const EventList& eventsToWaitFor = EventList());


        /**
         * Enqueue a synchronization point that ensures that all queued commands in this command queue have 
         * finished execution before the next batch of commands can begin execution. 
         * 
         * \todo    This method is deprecated in OpenCL 1.2
         */
        void enqueueBarrier();

        /**
         * Enqueues a marker to the command queue.
         * You can use the returned event for waiting for the command queue reach the marker.
         * 
         * \todo    This method is deprecated in OpenCL 1.2
         * \return  OpenCL event for waiting for the command queue reach the marker.
         */
        Event enqueueMarker();

        /**
         * Enqueues a wait for the given list of events to complete before any future commands of this command queue are executed.
         * 
         * \todo    This method is deprecated in OpenCL 1.2
         * \param   eventsToWaitFor List of Events to wait for, default is empty.
         */
        void enqueueWaitForEvents(const EventList& eventsToWaitFor = EventList());


        /**
         * Enqueues to read from a buffer object to host memory. 
         * \param   buffer          Buffer object to read from.
         * \param   data            The pointer to host memory where data is to be read into.
         * \param   blocking        Flag whether this operation shall be blocking or not. Defaults to true.
         * \param   offset          The offset in bytes in the buffer object to read from.
         * \param   numBytes        The number of bytes to be read. If 0, buffer->getSize() bytes will be read (default).
         * \param   eventsToWaitFor List of Events to wait for, default is empty.
         * \return  Event object that identifies this particular kernel execution instance.
         */
        Event enqueueRead(const Buffer* buffer, void* data, bool blocking = true, size_t offset = 0, size_t numBytes = 0, const EventList& eventsToWaitFor = EventList());

        /**
         * Enqueues to write into a buffer object from host memory. 
         * \param   buffer          Buffer object to write into.
         * \param   data            The pointer to host memory where data is read from.
         * \param   blocking        Flag whether this operation shall be blocking or not. Defaults to true.
         * \param   offset          The offset in bytes in the buffer object to start writing.
         * \param   numBytes        The number of bytes to be written. If 0, buffer->getSize() bytes will be written (default).
         * \param   eventsToWaitFor List of Events to wait for, default is empty.
         * \return  Event object that identifies this particular kernel execution instance.
         */
        Event enqueueWrite(const Buffer* buffer, void* data, bool blocking = true, size_t offset = 0, size_t numBytes = 0, const EventList& eventsToWaitFor = EventList());

        // TODO: buffers, images, etc.

        Event enqueueAcquireGLObject(const SharedTexture* texture, const EventList& eventsToWaitFor = EventList());

        Event enqueueReleaseGLObject(const SharedTexture* texture, const EventList& eventsToWaitFor = EventList());

    private:

        Context* _context;                      ///< OpenCL context for which the command queue is created.
        Device* _device;                        ///< OpenCL device for which the command queue is created.
        const bool _profilingEnabled;           ///< Flag whether the OpenCL command queue profiling is enabled (const since OpenCL 1.1).

        static const std::string loggerCat_;
    };
    
}

#endif // KISSCL_COMMANDQUEUE_H__

