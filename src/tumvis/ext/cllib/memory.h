#ifndef CLMEMORY_H__
#define CLMEMORY_H__

#include "cllib/cllib.h"



namespace cllib {
    class Context;

    /**
     * Base class for all OpenCL memory objects.
     */
    class MemoryObject : public CLWrapper<cl_mem> {
    public:
        /**
         * Creates a new OpenCL memory object.
         * \param context Context where this memory object resides.
         */
        MemoryObject(const Context* context);

        /**
         * Destructor, just hanging around virtually...
         */
        virtual ~MemoryObject();

        /**
         * Gets the context where this memory object resides.
         * \return _context
         */
        const Context* getContext() const;

    protected:
        const Context* _context;        ///< Context where this memory object resides.
    };

// ================================================================================================

    class Buffer : public MemoryObject {
    public:
        /**
         * Creates a new OpenCL buffer object.
         * \param   context     Context where this buffer object resides.
         * \param   flags       Flags specifying allocation and usage information for this buffer.
         * \param   size        The size in bytes of the buffer memory object to be allocated.
         * \param   hostPtr     A pointer to the buffer data for optional initialization of the buffer, defaults to 0.
         */
        Buffer(const Context* context, cl_mem_flags flags, size_t size, const void* hostPtr = 0);

        /**
         * Destructor, just hanging around virtually...
         */
        virtual ~Buffer();

        /**
         * Gets the size of this buffer
         * \return _size
         */
        size_t getSize() const;

    protected:
        size_t _size;       ///< Size of this buffer.
    };

// ================================================================================================

    class Image {
        // TODO: implement
    };
}

#endif // CLMEMORY_H__
