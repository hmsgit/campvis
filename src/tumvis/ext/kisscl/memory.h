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

#ifndef KISSCL_MEMORY_H__
#define KISSCL_MEMORY_H__

#include "tgt/vector.h"
#include "kisscl/kisscl.h"

namespace tgt {
    class Texture;
}

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

    /**
     * Simple wrapper around an OpenCL buffer.
     */
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

    /**
     * Wrapper around an OpenCL image that can be accessed in kernels via read/write_image.
     */
    class Image : public MemoryObject {
    public:
        /**
         * Creates an OpenCL image from a tgt::Texture.
         * \param   context     Context where this image object resides.
         * \param   flags       Flags specifying allocation and usage information for this buffer.
         * \param   texture     Pointer to the texture used for initalization, must have valid pixel data in host memory.
         */
        Image(const Context* context, cl_mem_flags flags, const tgt::Texture* texture);

        /**
         * Creates an OpenCL image from scratch.
         * \param   context         Context where this image object resides.
         * \param   flags           Flags specifying allocation and usage information for this buffer.
         * \param   dimensions      Image dimensions.
         * \param   channelOrder    OpenCL image channel order.
         * \param   channelType     OpenCL image channel data type.
         * \param   hostPtr         Pointer to the image data in the host memory.
         * \return 
         */
        Image(const Context* context, cl_mem_flags flags, const tgt::svec3& dimensions, cl_channel_order channelOrder, cl_channel_type channelType, void* hostPtr);

        /// Just a virtual dtor.
        virtual ~Image();
    };

    // ================================================================================================

    /**
     * Wrapper for an OpenCL image being shared with an OpenGL texture.
     */
    class GLTexture : public MemoryObject {
    public:
        /**
         * Creates an OpenCL shared GL image from a tgt::Texture.
         * \param   context     Context where this image object resides.
         * \param   flags       Flags specifying allocation and usage information for this buffer.
         * \param   texture     Pointer to the texture used for initalization, must refer to a valid and initialized OpenGL texture.
         */
        GLTexture(const Context* context, cl_mem_flags flags, const tgt::Texture* texture);

        /// Virtual dtor.
        virtual ~GLTexture();

    protected:
        const tgt::Texture* _texture;   ///< Pointer to the texture used for initalization, might be handy some time.
    };
}

#endif // KISSCL_MEMORY_H__
