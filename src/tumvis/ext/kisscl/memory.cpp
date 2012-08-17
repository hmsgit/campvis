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
