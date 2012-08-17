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

#ifndef KISSCL_MEMORY_H__
#define KISSCL_MEMORY_H__

#include "kisscl/kisscl.h"



namespace kisscl {
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

#endif // KISSCL_MEMORY_H__
