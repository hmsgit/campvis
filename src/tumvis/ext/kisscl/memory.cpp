#include "memory.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "kisscl/context.h"

namespace kisscl {
    
    MemoryObject::MemoryObject(const Context* context)
        : CLWrapper<cl_mem>(0)
        , _context(context)
    {
    }

    MemoryObject::~MemoryObject() {

    }

    const Context* MemoryObject::getContext() const {
        return _context;
    }

// ================================================================================================

    Buffer::Buffer(const Context* context, cl_mem_flags flags, size_t size, const void* hostPtr /*= 0*/)
        : MemoryObject(context)
        , _size(size)
    {
        tgtAssert(size > 0, "Buffer size must be greater than 0.");

        cl_int err;
        _id = clCreateBuffer(context->getId(), flags, size, const_cast<void*>(hostPtr), &err);
        LCL_ERROR(err);
    }


    Buffer::~Buffer() {

    }

    size_t Buffer::getSize() const {
        return _size;
    }

}
