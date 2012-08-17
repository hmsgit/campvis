#ifndef COMMANDQUEUE_H__
#define COMMANDQUEUE_H__

#include "tgt/vector.h"
#include "cllib/cllib.h"
#include "cllib/event.h"

#include <vector>


namespace cllib {
    class Context;
    class Device;
    class Kernel;
    
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


        // TODO: buffers, images, etc.

    private:

        Context* _context;                      ///< OpenCL context for which the command queue is created.
        Device* _device;                        ///< OpenCL device for which the command queue is created.
        const bool _profilingEnabled;           ///< Flag whether the OpenCL command queue profiling is enabled (const since OpenCL 1.1).

        static const std::string loggerCat_;
    };
    
}

#endif // COMMANDQUEUE_H__

