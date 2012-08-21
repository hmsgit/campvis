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
#include "tgt/texture.h"
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

// ================================================================================================

    Image::Image(const Context* context, cl_mem_flags flags, const tgt::Texture* texture)
        : MemoryObject(context)
    {
        tgtAssert(texture != 0, "Texture must not be 0");
        if (!texture->getPixelData()) {
            LERRORC("kisscl.Image", "Invalid pixel data in given texture.");
            return;
        }

        cl_int err;
        const tgt::ivec3& dims = texture->getDimensions();
        GLenum texDataType = texture->getDataType();
        GLint  texFormat = texture->getFormat();

        cl_image_format imageFormat;
        switch(texDataType) {
            case GL_BYTE:
                imageFormat.image_channel_data_type = CL_SNORM_INT8;
                break;
            case GL_UNSIGNED_BYTE:
                imageFormat.image_channel_data_type = CL_UNORM_INT8;
                break;
            case GL_SHORT:
                imageFormat.image_channel_data_type = CL_SNORM_INT16;
                break;
            case GL_UNSIGNED_SHORT:
                imageFormat.image_channel_data_type = CL_UNORM_INT16;
                break;
            case GL_INT:
                imageFormat.image_channel_data_type = CL_SIGNED_INT32;
                break;
            case GL_UNSIGNED_INT:
                imageFormat.image_channel_data_type = CL_UNSIGNED_INT32;
                break;
            case GL_FLOAT:
                imageFormat.image_channel_data_type = CL_FLOAT;
                break;
        }

        switch(texFormat) {
            case GL_ALPHA:
            case GL_LUMINANCE:
                imageFormat.image_channel_order = CL_INTENSITY;
                break;
            case GL_LUMINANCE_ALPHA:
                imageFormat.image_channel_order = CL_RA;
                break;
            case GL_RGBA:
                imageFormat.image_channel_order = CL_RGBA;
                break;
        }

#if defined(CL_VERSION_1_2)
        cl_image_desc desc;
        if (dims.y == 1 && dims.z == 1)
            desc.image_type = CL_MEM_OBJECT_IMAGE1D;
        else if (dims.z == 1)
            desc.image_type = CL_MEM_OBJECT_IMAGE2D;
        else
            desc.image_type = CL_MEM_OBJECT_IMAGE3D;

        desc.image_width = dims.x;
        desc.image_height = dims.y;
        desc.image_depth = dims.z
        desc.image_row_pitch = 0;
        desc.image_slice_pitch = 0;
        desc.num_mip_levels = 0;
        desc.num_samples = 0;
        desc.buffer = 0;

        object_ = clCreateImage(context->getId(), flags, &imageFormat, &desc, texture->getPixelData(), &err);
#else // defined(CL_VERSION_1_2)
        if (dims.z == 1) {
            // 1D or 2D image
            _id = clCreateImage2D(context->getId(), flags, &imageFormat, dims.x, dims.y, 0, const_cast<GLubyte*>(texture->getPixelData()), &err);
        }
        else {
            // 3D image
            _id = clCreateImage3D(context->getId(), flags, &imageFormat, dims.x, dims.y, dims.z, 0, 0, const_cast<GLubyte*>(texture->getPixelData()), &err);
        }
#endif // defined(CL_VERSION_1_2)

        LCL_ERROR(err);
    }

    Image::Image(const Context* context, cl_mem_flags flags, const tgt::svec3& dimensions, cl_channel_order channelOrder, cl_channel_type channelType, void* hostPtr)
        : MemoryObject(context)
    {
        tgtAssert(hostPtr != 0, "Texture must not be 0");

        cl_int err;
        cl_image_format imageFormat;
        imageFormat.image_channel_data_type = channelType;
        imageFormat.image_channel_order = channelOrder;

#if defined(CL_VERSION_1_2)
        cl_image_desc desc;
        if (dimensions.y == 1 && dimensions.z == 1)
            desc.image_type = CL_MEM_OBJECT_IMAGE1D;
        else if (dimensions.z == 1)
            desc.image_type = CL_MEM_OBJECT_IMAGE2D;
        else
            desc.image_type = CL_MEM_OBJECT_IMAGE3D;

        desc.image_width = dimensions.x;
        desc.image_height = dimensions.y;
        desc.image_depth = dimensions.z
            desc.image_row_pitch = 0;
        desc.image_slice_pitch = 0;
        desc.num_mip_levels = 0;
        desc.num_samples = 0;
        desc.buffer = 0;

        object_ = clCreateImage(context->getId(), flags, &imageFormat, &desc, texture->getPixelData(), &err);
#else // defined(CL_VERSION_1_2)
        if (dimensions.z == 1) {
            // 1D or 2D image
            _id = clCreateImage2D(context->getId(), flags, &imageFormat, dimensions.x, dimensions.y, 0, hostPtr, &err);
        }
        else {
            // 3D image
            _id = clCreateImage3D(context->getId(), flags, &imageFormat, dimensions.x, dimensions.y, dimensions.z, 0, 0, hostPtr, &err);
        }
#endif // defined(CL_VERSION_1_2)

        LCL_ERROR(err);
    }

    Image::~Image() {

    }


    SharedTexture::SharedTexture(const Context* context, cl_mem_flags flags, const tgt::Texture* texture)
        : MemoryObject(context)
        , _texture(texture)
    {
        tgtAssert(_texture != 0, "Texture must not be 0.");
        tgtAssert(glIsTexture(_texture->getId()) == GL_TRUE, "Texture must be a valid OpenGL texture.");

        cl_int err;
#if defined(CL_VERSION_1_2)
        _id = clCreateFromGLTexture(context->getId(), flags, _texture->getType(), 0, _texture->getId(), &err);
#else // defined(CL_VERSION_1_2)
        switch(_texture->getType()) {
            case GL_TEXTURE_3D:
                _id = clCreateFromGLTexture3D(context->getId(), flags, _texture->getType(), 0, _texture->getId(), &err);
                LCL_ERROR(err);
                break;
            case GL_TEXTURE_2D:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_X:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Y:
            case GL_TEXTURE_CUBE_MAP_POSITIVE_Z:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_X:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Y:
            case GL_TEXTURE_CUBE_MAP_NEGATIVE_Z:
            case GL_TEXTURE_RECTANGLE:
                _id = clCreateFromGLTexture2D(context->getId(), flags, _texture->getType(), 0, _texture->getId(), &err);
                LCL_ERROR(err);
                break;
            default:
                LERRORC("kisscl.SharedTexture", "Unknown texture type!");
#endif // defined(CL_VERSION_1_2)

        }
    }

    SharedTexture::~SharedTexture() {

    }

}
