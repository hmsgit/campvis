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
//  * The names of its contributors may not be used to endorse or promote products derived from
//    this software without specific prior written permission.
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

#ifndef KISSCL_COMMANDQUEUE_H__
#define KISSCL_COMMANDQUEUE_H__

#include "tgt/vector.h"
#include "kisscl/kisscl.h"
#include "kisscl/itemlist.h"
#include "kisscl/event.h"

#include <vector>


namespace kisscl {
    class Buffer;
    class Context;
    class Device;
    class Kernel;
    class GLTexture;

    typedef ItemList<Event> EventList;
    typedef ItemList<GLTexture> GLTextureList;

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
        Event enqueueTask(const Kernel* kernel, const ItemList<Event>& eventsToWaitFor = ItemList<Event>());

        /**
         * Enqueues an one-dimensional OpenCL kernel to this command queue.
         * \param   kernel          The kernel to execute.
         * \param   globalWorkSize  Global work size for the kernel (number of work-items).
         * \param   localWorkSize   Local work size (work group size), defaults 0 to let the OpenCL implementation choose (not recommended).
         * \param   offset          Offset to calculate the global ID of a work-item, default is 0.
         * \param   eventsToWaitFor List of Events to wait for, default is empty.
         * \return  Event object that identifies this particular kernel execution instance.
         */
        Event enqueueKernel(const Kernel* kernel, size_t globalWorkSize, size_t localWorkSize = 0, size_t offset = 0, const ItemList<Event>& eventsToWaitFor = ItemList<Event>());

        /**
         * Enqueues a two-dimensional OpenCL kernel to this command queue.
         * \param   kernel          The kernel to execute.
         * \param   globalWorkSize  Global work size for the kernel (number of work-items).
         * \param   localWorkSize   Local work size (work group size), defaults to (0, 0) to let the OpenCL implementation choose (not recommended).
         * \param   offset          Offset to calculate the global ID of a work-item, default is (0, 0).
         * \param   eventsToWaitFor List of Events to wait for, default is empty.
         * \return  Event object that identifies this particular kernel execution instance.
         */
        Event enqueueKernel(const Kernel* kernel, tgt::svec2 globalWorkSize, tgt::svec2 localWorkSize = tgt::svec2::zero, tgt::svec2 offset = tgt::svec2::zero, const ItemList<Event>& eventsToWaitFor = ItemList<Event>());

        /**
         * Enqueues a three-dimensional OpenCL kernel to this command queue.
         * \param   kernel          The kernel to execute.
         * \param   globalWorkSize  Global work size for the kernel (number of work-items).
         * \param   localWorkSize   Local work size (work group size), defaults to (0, 0, 0) to let the OpenCL implementation choose (not recommended).
         * \param   offset          Offset to calculate the global ID of a work-item, default is (0, 0, 0).
         * \param   eventsToWaitFor List of Events to wait for, default is empty.
         * \return  Event object that identifies this particular kernel execution instance.
         */
        Event enqueueKernel(const Kernel* kernel, tgt::svec3 globalWorkSize, tgt::svec3 localWorkSize = tgt::svec3::zero, tgt::svec3 offset = tgt::svec3::zero, const ItemList<Event>& eventsToWaitFor = ItemList<Event>());


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
        void enqueueWaitForEvents(const ItemList<Event>& eventsToWaitFor = ItemList<Event>());


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
        Event enqueueRead(const Buffer* buffer, void* data, bool blocking = true, size_t offset = 0, size_t numBytes = 0, const ItemList<Event>& eventsToWaitFor = ItemList<Event>());

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
        Event enqueueWrite(const Buffer* buffer, void* data, bool blocking = true, size_t offset = 0, size_t numBytes = 0, const ItemList<Event>& eventsToWaitFor = ItemList<Event>());

        // TODO: buffers, images, etc.

        Event enqueueAcquireGLObject(const ItemList<GLTexture>& textures, const ItemList<Event>& eventsToWaitFor = ItemList<Event>());

        Event enqueueReleaseGLObject(const ItemList<GLTexture>& textures, const ItemList<Event>& eventsToWaitFor = ItemList<Event>());

    private:

        Context* _context;                      ///< OpenCL context for which the command queue is created.
        Device* _device;                        ///< OpenCL device for which the command queue is created.
        const bool _profilingEnabled;           ///< Flag whether the OpenCL command queue profiling is enabled (const since OpenCL 1.1).

        static const std::string loggerCat_;
    };
    
}

#endif // KISSCL_COMMANDQUEUE_H__

