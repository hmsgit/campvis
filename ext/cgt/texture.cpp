/**********************************************************************
 *                                                                    *
 * tgt - Tiny Graphics Toolbox                                        *
 *                                                                    *
 * Copyright (C) 2006-2011 Visualization and Computer Graphics Group, *
 * Department of Computer Science, University of Muenster, Germany.   *
 * <http://viscg.uni-muenster.de>                                     *
 *                                                                    *
 * This file is part of the tgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#include "cgt/texture.h"
#include "cgt/types.h"

#include "cgt/gpucapabilities.h"
#include "cgt/filesystem.h"
#include "cgt/openglgarbagecollector.h"

namespace tgt {

//------------------------------------------------------------------------------
// Texture
//------------------------------------------------------------------------------

Texture::Texture(const tgt::ivec3& dimensions, GLint format, GLint internalformat,
                GLenum dataType, Filter filter)
    : dimensions_(dimensions)
    , format_(format)
    , internalformat_(internalformat)
    , dataType_(dataType)
    , filter_(filter)
    , wrapping_(CLAMP)
    , priority_(-1.f)
    , pixels_(0)
{
    init(true);
}

Texture::Texture(const tgt::ivec3& dimensions, GLint format,
                GLenum dataType, Filter filter)
    : dimensions_(dimensions)
    , format_(format)
    , internalformat_(format)
    , dataType_(dataType)
    , filter_(filter)
    , wrapping_(CLAMP)
    , priority_(-1.f)
    , pixels_(0)
{
    init(true);
}

Texture::Texture(GLubyte* data, const tgt::ivec3& dimensions, GLint format, GLint internalformat,
                GLenum dataType, Filter filter)
    : dimensions_(dimensions)
    , format_(format)
    , internalformat_(internalformat)
    , dataType_(dataType)
    , filter_(filter)
    , wrapping_(CLAMP)
    , priority_(-1.f)
    , pixels_(data)
{
    init(false);
    arraySize_ = hmul(dimensions_) * bpp_;
}

Texture::Texture(GLubyte* data, const tgt::ivec3& dimensions, GLint format,
                GLenum dataType, Filter filter)
    : dimensions_(dimensions)
    , format_(format)
    , internalformat_(format)
    , dataType_(dataType)
    , filter_(filter)
    , wrapping_(CLAMP)
    , priority_(-1.f)
    , pixels_(data)
{
    init(false);
    arraySize_ = hmul(dimensions_) * bpp_;
}

Texture::~Texture() {
    if (id_)
        GLGC.addGarbageTexture(id_);

    if (pixels_)
        delete[] pixels_;
}


void Texture::init(bool allocData) {
#ifndef GL_TEXTURE_RECTANGLE_ARB
    textureRectangle = false;
#endif

    calcType();
    bpp_ = calcBpp(format_, dataType_);

    generateId();

    if (allocData)
        alloc();

    applyFilter();
}

int Texture::calcBpp(GLint format, GLenum dataType) {

    int numChannels = calcNumChannels(format);

    int typeSize = 0;
    switch (dataType) {
        case GL_BYTE:
        case GL_UNSIGNED_BYTE:
            typeSize = 1;
            break;

        case GL_SHORT:
        case GL_UNSIGNED_SHORT:
            typeSize = 2;
            break;

        case GL_INT:
        case GL_UNSIGNED_INT:
        case GL_FLOAT:
            typeSize = 4;
            break;

        default:
            LWARNINGC("tgt.Texture", "unknown dataType");
    }

    return typeSize * numChannels;
}

int Texture::calcBpp(GLint internalformat) {
    // supports all formats from http://www.opengl.org/sdk/docs/man/xhtml/glTexImage2D.xml

    int bpp = 0;
    switch (internalformat) {
        case 1:
        case GL_DEPTH_COMPONENT:
        case GL_RED:
        case GL_R8:
        case GL_R8_SNORM:
        case GL_R8I:
        case GL_R8UI:
        case GL_R3_G3_B2:
        case GL_RGBA2:
            bpp = 1;
            break;

        case 2:
        case GL_DEPTH_COMPONENT16:
        case GL_R16_SNORM:
        case GL_R16F:
        case GL_R16I:
        case GL_R16UI:
        case GL_DEPTH_STENCIL:
        case GL_RG:
        case GL_RG8:
        case GL_RG8_SNORM:
        case GL_RG8I:
        case GL_RG8UI:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGBA4:
        case GL_RGB5_A1:
            bpp = 2;
            break;

        case GL_DEPTH_COMPONENT24:
        case GL_RGB:
        case GL_RGB8:
        case GL_RGB8_SNORM:
        case GL_SRGB8:
        case GL_RGB8I:
        case GL_RGB8UI:
            bpp = 3;
            break;

        case GL_DEPTH_COMPONENT32:
        case GL_DEPTH_COMPONENT32F:
        case GL_R32F:
        case GL_R32I:
        case GL_R32UI:
        case GL_RG16:
        case GL_RG16_SNORM:
        case GL_RG16F:
        case GL_RG16I:
        case GL_RG16UI:
        case GL_RGB10:
        case GL_R11F_G11F_B10F:
        case GL_RGB9_E5:
        case GL_RGBA:
        case GL_RGBA8:
        case GL_RGBA8_SNORM:
        case GL_RGB10_A2:
        case GL_RGB10_A2UI:
        case GL_SRGB8_ALPHA8:
        case GL_RGBA8I:
        case GL_RGBA8UI:
            bpp = 4;
            break;

        case GL_RGB12:
            bpp = 5;
            break;

        case GL_RGB16_SNORM:
        case GL_RGB16F:
        case GL_RGB16I:
        case GL_RGB16UI:
        case GL_RGBA12:
            bpp = 6;
            break;

        case GL_RG32F:
        case GL_RG32I:
        case GL_RG32UI:
        case GL_RGBA16:
        case GL_RGBA16F:
        case GL_RGBA16I:
        case GL_RGBA16UI:
            bpp = 8;
            break;

        case GL_RGB32F:
        case GL_RGB32I:
        case GL_RGB32UI:
            bpp = 12;
            break;

        case GL_RGBA32F:
        case GL_RGBA32I:
        case GL_RGBA32UI:
            bpp = 16;
            break;

        default:
            LWARNINGC("tgt.Texture", "unknown internal format");
            break;
    }

    return bpp;
}

int Texture::calcNumChannels(GLint format) {
    // supports all formats from http://www.opengl.org/sdk/docs/man/xhtml/glTexImage2D.xml
    switch (format) {
        case 1:
        case GL_DEPTH_COMPONENT:
        case GL_RED:
        case GL_R8:
        case GL_R8_SNORM:
        case GL_R16_SNORM:
        case GL_R16F:
        case GL_R32F:
        case GL_R8I:
        case GL_R8UI:
        case GL_R16I:
        case GL_R16UI:
        case GL_R32I:
        case GL_R32UI:
        case GL_RED_INTEGER:
            return 1;
            break;

        case 2:
        case GL_DEPTH_STENCIL:
        case GL_RG:
        case GL_RG8:
        case GL_RG8_SNORM:
        case GL_RG16:
        case GL_RG16_SNORM:
        case GL_RG16F:
        case GL_RG32F:
        case GL_RG8I:
        case GL_RG8UI:
        case GL_RG16I:
        case GL_RG16UI:
        case GL_RG32I:
        case GL_RG32UI:
        case GL_RG_INTEGER:
            return 2;
            break;

        case 3:
        case GL_RGB:
        case GL_R3_G3_B2:
        case GL_RGB4:
        case GL_RGB5:
        case GL_RGB8:
        case GL_RGB8_SNORM:
        case GL_RGB10:
        case GL_RGB12:
        case GL_RGB16_SNORM:
        case GL_SRGB8:
        case GL_RGB16F:
        case GL_RGB32F:
        case GL_R11F_G11F_B10F:
        case GL_RGB9_E5:
        case GL_RGB8I:
        case GL_RGB8UI:
        case GL_RGB16I:
        case GL_RGB16UI:
        case GL_RGB32I:
        case GL_RGB32UI:
        case GL_RGB_INTEGER:
            return 3;
            break;

        case 4:
        case GL_RGBA:
        case GL_RGBA2:
        case GL_RGBA4:
        case GL_RGB5_A1:
        case GL_RGBA8:
        case GL_RGBA8_SNORM:
        case GL_RGB10_A2:
        case GL_RGB10_A2UI:
        case GL_RGBA12:
        case GL_RGBA16:
        case GL_SRGB8_ALPHA8:
        case GL_RGBA16F:
        case GL_RGBA32F:
        case GL_RGBA8I:
        case GL_RGBA8UI:
        case GL_RGBA16I:
        case GL_RGBA16UI:
        case GL_RGBA32I:
        case GL_RGBA32UI:
        case GL_RGBA_INTEGER:
            return 4;
            break;

        default:
            LWARNINGC("tgt.Texture", "unknown format");
            return 0;
    }
}

int Texture::getSizeOnGPU() const {
    int bpp = calcBpp(internalformat_);
    return bpp * hmul(dimensions_);
}

GLenum Texture::calcType(bool textureRectangle) {
    if (dimensions_.z == 1)    {
        if (dimensions_.y == 1)
            type_ = GL_TEXTURE_1D;
        else
            type_ = GL_TEXTURE_2D;
    }
    else {
        type_ = GL_TEXTURE_3D;
    }

#ifdef GL_TEXTURE_RECTANGLE_ARB
    if (type_ == GL_TEXTURE_2D && textureRectangle)
        type_ = GL_TEXTURE_RECTANGLE_ARB;
#endif

    return type_;
}

void Texture::setFilter(Filter filter) {
    filter_ = filter;
    applyFilter();
}

void Texture::applyFilter() {
    bind();

    switch(filter_) {
        case NEAREST:
            glTexParameteri(type_,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
            glTexParameteri(type_,GL_TEXTURE_MIN_FILTER,GL_NEAREST);
            break;

        case LINEAR:
            glTexParameteri(type_, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
            glTexParameteri(type_, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
            break;

        case ANISOTROPIC:
            glTexParameterf(type_, GL_TEXTURE_MAX_ANISOTROPY_EXT, GpuCaps.getMaxTextureAnisotropy());

        case MIPMAP:
            glTexParameteri(type_,GL_TEXTURE_MAG_FILTER,GL_NEAREST);
            glTexParameteri(type_,GL_TEXTURE_MIN_FILTER,GL_NEAREST_MIPMAP_NEAREST);
            break;
    }
}

void Texture::setWrapping(Wrapping w) {
    wrapping_ = w;
    applyWrapping();
}

void Texture::applyWrapping() {
    bind();
    GLint wrap = wrapping_;

    /*
        set wrapping for all diminesions
    */
    glTexParameteri(type_, GL_TEXTURE_WRAP_S, wrap);

    if (type_ == GL_TEXTURE_2D || type_ == GL_TEXTURE_3D)
        glTexParameteri(type_, GL_TEXTURE_WRAP_T, wrap);
    if (GL_TEXTURE_3D)
        glTexParameteri(type_, GL_TEXTURE_WRAP_R, wrap);
}

void Texture::uploadTexture() {
    bind();

    switch(type_) {
        case GL_TEXTURE_1D:
            glTexImage1D(GL_TEXTURE_1D, 0, internalformat_, dimensions_.x, 0,
                         format_, dataType_, pixels_);
            if (filter_ == MIPMAP)
                glGenerateMipmap(GL_TEXTURE_1D);
            break;

        case GL_TEXTURE_2D:
            glTexImage2D(GL_TEXTURE_2D, 0, internalformat_, dimensions_.x, dimensions_.y, 0,
                         format_, dataType_, pixels_);
            if (filter_ == MIPMAP)
                glGenerateMipmap(GL_TEXTURE_2D);
            break;

        case GL_TEXTURE_3D:
            glTexImage3D(GL_TEXTURE_3D, 0, internalformat_,
                         dimensions_.x, dimensions_.y, dimensions_.z, 0,
                         format_, dataType_, pixels_);
            if (filter_ == MIPMAP)
                glGenerateMipmap(GL_TEXTURE_3D);
            break;

#ifdef GL_TEXTURE_RECTANGLE_ARB
        case GL_TEXTURE_RECTANGLE_ARB:
            glTexImage2D(GL_TEXTURE_RECTANGLE_ARB, 0, internalformat_, dimensions_.x, dimensions_.y, 0,
                            format_, dataType_, pixels_);
            break;
#endif
    }

    LGL_ERROR;
}

tgt::vec4 Texture::texelAsFloat(size_t x, size_t y) const {
    tgt::vec4 ret = tgt::vec4(0.0f);

    switch (getNumChannels()) {
        case 4:
            switch(dataType_) {
                case GL_BYTE:
                    ret = tgt::vec4(texel< tgt::Vector4<int8_t> >(x, y)) / float(0xFF);
                    break;
                case GL_UNSIGNED_BYTE:
                    ret = tgt::vec4(texel< tgt::Vector4<uint8_t> >(x, y)) / float(0xFF);
                    break;
                case GL_SHORT:
                    ret = tgt::vec4(texel< tgt::Vector4<int16_t> >(x, y)) / float(0xFFFF);
                    break;
                case GL_UNSIGNED_SHORT:
                    ret = tgt::vec4(texel< tgt::Vector4<uint16_t> >(x, y)) / float(0xFFFF);
                    break;
                case GL_INT:
                    ret = tgt::vec4(texel< tgt::Vector4<int32_t> >(x, y)) / float(0xFFFFFFFF);
                    break;
                case GL_UNSIGNED_INT:
                    ret = tgt::vec4(texel< tgt::Vector4<uint32_t> >(x, y)) / float(0xFFFFFFFF);
                    break;
                case GL_FLOAT:
                    ret = texel< tgt::vec4 >(x, y);
                    break;
                default:
                    LWARNINGC("tgt.texture", "texelAsFloat: Unknown data type!");
            }
            break;

        case 3: {
            tgt::vec3 tmp;
            switch(dataType_) {
                case GL_BYTE:
                    tmp = tgt::vec3(texel< tgt::Vector3<int8_t> >(x, y)) / float(0xFF);
                    break;
                case GL_UNSIGNED_BYTE:
                    tmp = tgt::vec3(texel< tgt::Vector3<uint8_t> >(x, y)) / float(0xFF);
                    break;
                case GL_SHORT:
                    tmp = tgt::vec3(texel< tgt::Vector3<int16_t> >(x, y)) / float(0xFFFF);
                    break;
                case GL_UNSIGNED_SHORT:
                    tmp = tgt::vec3(texel< tgt::Vector3<uint16_t> >(x, y)) / float(0xFFFF);
                    break;
                case GL_INT:
                    tmp = tgt::vec3(texel< tgt::Vector3<int32_t> >(x, y)) / float(0xFFFFFFFF);
                    break;
                case GL_UNSIGNED_INT:
                    tmp = tgt::vec3(texel< tgt::Vector3<uint32_t> >(x, y)) / float(0xFFFFFFFF);
                    break;
                case GL_FLOAT:
                    tmp = texel< tgt::vec3 >(x, y);
                    break;
                default:
                    LWARNINGC("tgt.texture", "texelAsFloat: Unknown data type!");
                    break;
            }
            ret.x = tmp.x;
            ret.y = tmp.y;
            ret.z = tmp.z;
            break;
        }

        case 2: {
            tgt::vec2 tmp;
            switch(dataType_) {
                case GL_BYTE:
                    tmp = tgt::vec2(texel< tgt::Vector2<int8_t> >(x, y)) / float(0xFF);
                    break;
                case GL_UNSIGNED_BYTE:
                    tmp = tgt::vec2(texel< tgt::Vector2<uint8_t> >(x, y)) / float(0xFF);
                    break;
                case GL_SHORT:
                    tmp = tgt::vec2(texel< tgt::Vector2<int16_t> >(x, y)) / float(0xFFFF);
                    break;
                case GL_UNSIGNED_SHORT:
                    tmp = tgt::vec2(texel< tgt::Vector2<uint16_t> >(x, y)) / float(0xFFFF);
                    break;
                case GL_INT:
                    tmp = tgt::vec2(texel< tgt::Vector2<int32_t> >(x, y)) / float(0xFFFFFFFF);
                    break;
                case GL_UNSIGNED_INT:
                    tmp = tgt::vec2(texel< tgt::Vector2<uint32_t> >(x, y)) / float(0xFFFFFFFF);
                    break;
                case GL_FLOAT:
                    tmp = texel< tgt::vec2 >(x, y);
                    break;
                default:
                    LWARNINGC("tgt.texture", "texelAsFloat: Unknown data type!");
                    break;
            }
            ret.x = tmp.x;
            ret.y = tmp.y;
            break;
        }

        case 1:
            switch(dataType_) {
                case GL_BYTE:
                    ret.x = static_cast<float>(texel< int8_t >(x, y)) / float(0xFF);
                    break;
                case GL_UNSIGNED_BYTE:
                    ret.x = static_cast<float>(texel< uint8_t >(x, y)) / float(0xFF);
                    break;
                case GL_SHORT:
                    ret.x = static_cast<float>(texel< int16_t >(x, y)) / float(0xFFFF);
                    break;
                case GL_UNSIGNED_SHORT:
                    ret.x = static_cast<float>(texel< uint16_t >(x, y)) / float(0xFFFF);
                    break;
                case GL_INT:
                    ret.x = static_cast<float>(texel< int32_t >(x, y)) / float(0xFFFFFFFF);
                    break;
                case GL_UNSIGNED_INT:
                    ret.x = static_cast<float>(texel< uint32_t >(x, y)) / float(0xFFFFFFFF);
                    break;
                case GL_FLOAT:
                    ret.x = texel<float>(x, y);
                    break;
                default:
                    LWARNINGC("tgt.texture", "texelAsFloat: Unknown data type!");
            }
            break;

        default:
            LWARNINGC("tgt.texture", "texelAsFloat: Unknown format!");
    }

    return ret;
}


tgt::vec4 Texture::texelAsFloat(size_t x, size_t y, size_t z) const {
    tgt::vec4 ret = tgt::vec4(0.0f);

    switch (getNumChannels()) {
        case 4:
            switch(dataType_) {
                case GL_BYTE:
                    ret = tgt::vec4(texel< tgt::Vector4<int8_t> >(x,y,z)) / float(0xFF);
                    break;
                case GL_UNSIGNED_BYTE:
                    ret = tgt::vec4(texel< tgt::Vector4<uint8_t> >(x,y,z)) / float(0xFF);
                    break;
                case GL_SHORT:
                    ret = tgt::vec4(texel< tgt::Vector4<int16_t> >(x,y,z)) / float(0xFFFF);
                    break;
                case GL_UNSIGNED_SHORT:
                    ret = tgt::vec4(texel< tgt::Vector4<uint16_t> >(x,y,z)) / float(0xFFFF);
                    break;
                case GL_INT:
                    ret = tgt::vec4(texel< tgt::Vector4<int32_t> >(x,y,z)) / float(0xFFFFFFFF);
                    break;
                case GL_UNSIGNED_INT:
                    ret = tgt::vec4(texel< tgt::Vector4<uint32_t> >(x,y,z)) / float(0xFFFFFFFF);
                    break;
                case GL_FLOAT:
                    ret = texel< tgt::vec4 >(x,y,z);
                    break;
                default:
                    LWARNINGC("tgt.texture", "texelAsFloat: Unknown data type!");
            }
            break;

        case 3: {
            tgt::vec3 tmp;
            switch(dataType_) {
                case GL_BYTE:
                    tmp = tgt::vec3(texel< tgt::Vector3<int8_t> >(x,y,z)) / float(0xFF);
                    break;
                case GL_UNSIGNED_BYTE:
                    tmp = tgt::vec3(texel< tgt::Vector3<uint8_t> >(x,y,z)) / float(0xFF);
                    break;
                case GL_SHORT:
                    tmp = tgt::vec3(texel< tgt::Vector3<int16_t> >(x,y,z)) / float(0xFFFF);
                    break;
                case GL_UNSIGNED_SHORT:
                    tmp = tgt::vec3(texel< tgt::Vector3<uint16_t> >(x,y,z)) / float(0xFFFF);
                    break;
                case GL_INT:
                    tmp = tgt::vec3(texel< tgt::Vector3<int32_t> >(x,y,z)) / float(0xFFFFFFFF);
                    break;
                case GL_UNSIGNED_INT:
                    tmp = tgt::vec3(texel< tgt::Vector3<uint32_t> >(x,y,z)) / float(0xFFFFFFFF);
                    break;
                case GL_FLOAT:
                    tmp = texel< tgt::vec3 >(x,y,z);
                    break;
                default:
                    LWARNINGC("tgt.texture", "texelAsFloat: Unknown data type!");
                    break;
            }
            ret.x = tmp.x;
            ret.y = tmp.y;
            ret.z = tmp.z;
            break;
        }

        case 2: {
            tgt::vec2 tmp;
            switch(dataType_) {
                case GL_BYTE:
                    tmp = tgt::vec2(texel< tgt::Vector2<int8_t> >(x,y,z)) / float(0xFF);
                    break;
                case GL_UNSIGNED_BYTE:
                    tmp = tgt::vec2(texel< tgt::Vector2<uint8_t> >(x,y,z)) / float(0xFF);
                    break;
                case GL_SHORT:
                    tmp = tgt::vec2(texel< tgt::Vector2<int16_t> >(x,y,z)) / float(0xFFFF);
                    break;
                case GL_UNSIGNED_SHORT:
                    tmp = tgt::vec2(texel< tgt::Vector2<uint16_t> >(x,y,z)) / float(0xFFFF);
                    break;
                case GL_INT:
                    tmp = tgt::vec2(texel< tgt::Vector2<int32_t> >(x,y,z)) / float(0xFFFFFFFF);
                    break;
                case GL_UNSIGNED_INT:
                    tmp = tgt::vec2(texel< tgt::Vector2<uint32_t> >(x,y,z)) / float(0xFFFFFFFF);
                    break;
                case GL_FLOAT:
                    tmp = texel< tgt::vec2 >(x,y,z);
                    break;
                default:
                    LWARNINGC("tgt.texture", "texelAsFloat: Unknown data type!");
                    break;
            }
            ret.x = tmp.x;
            ret.y = tmp.y;
            break;
        }

        case 1:
            switch(dataType_) {
                case GL_BYTE:
                    ret.x = static_cast<float>(texel< int8_t >(x,y,z)) / float(0xFF);
                    break;
                case GL_UNSIGNED_BYTE:
                    ret.x = static_cast<float>(texel< uint8_t >(x,y,z)) / float(0xFF);
                    break;
                case GL_SHORT:
                    ret.x = static_cast<float>(texel< int16_t >(x,y,z)) / float(0xFFFF);
                    break;
                case GL_UNSIGNED_SHORT:
                    ret.x = static_cast<float>(texel< uint16_t >(x,y,z)) / float(0xFFFF);
                    break;
                case GL_INT:
                    ret.x = static_cast<float>(texel< int32_t >(x,y,z)) / float(0xFFFFFFFF);
                    break;
                case GL_UNSIGNED_INT:
                    ret.x = static_cast<float>(texel< uint32_t >(x,y,z)) / float(0xFFFFFFFF);
                    break;
                case GL_FLOAT:
                    ret.x = texel<float>(x,y,z);
                    break;
                default:
                    LWARNINGC("tgt.texture", "texelAsFloat: Unknown data type!");
            }
            break;

        default:
            LWARNINGC("tgt.texture", "texelAsFloat: Unknown format!");
    }

    return ret;
}

float Texture::depthAsFloat(size_t x, size_t y) const {
    float ret = 0.0f;

    switch (format_) {
        case GL_DEPTH_COMPONENT:    // fallthrough
        case GL_DEPTH_COMPONENT16:  // fallthrough
        case GL_DEPTH_COMPONENT24:  // fallthrough
        case GL_DEPTH_COMPONENT32:  // fallthrough
        case GL_DEPTH_COMPONENT32F:
            switch (dataType_) {
                case GL_UNSIGNED_BYTE: {
                    ret = (float )(texel<uint8_t>(x,y) / 0xFF);
                    break;
                }
                case GL_UNSIGNED_SHORT: {
                    ret = (float )(texel<uint16_t>(x,y) / 0xFFFF);
                    break;
                }
                case GL_FLOAT: {
                    ret = texel<GLfloat>(x,y);
                    break;
                }
                default:
                    LWARNINGC("tgt.texture", "depthAsFloat: Unknown format!");
            }
            break;
        default:
            LWARNINGC("tgt.texture", "depthAsFloat: Unknown format!");
            break;
    }

    return ret;
}

void Texture::downloadTexture() {
    bind();

    if (pixels_ == 0)
        alloc();

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glGetTexImage(type_, 0, format_, dataType_, pixels_);
}

GLubyte* Texture::downloadTextureToBuffer() const {
    bind();

    int arraySize = hmul(dimensions_) * bpp_;
    GLubyte* pixels = new GLubyte[arraySize];

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glGetTexImage(type_, 0, format_, dataType_, pixels);
    return pixels;
}

void Texture::downloadTextureToBuffer(GLubyte* pixels, size_t numBytesAllocated) const {
    bind();

    size_t arraySize = hmul(dimensions_) * bpp_;
    if(numBytesAllocated < arraySize) {
        LWARNINGC("tgt.texture", "downloadTextureToBuffer: allocated buffer is too small");
    }
    else {
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
        glGetTexImage(type_, 0, format_, dataType_, pixels);
    }
}

GLubyte* Texture::downloadTextureToBuffer(GLint format, GLenum dataType) const {
    bind();

    int arraySize = hmul(dimensions_) * calcBpp(format, dataType);
    GLubyte* pixels = new GLubyte[arraySize];

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glGetTexImage(type_, 0, format, dataType, pixels);
    return pixels;
}

bool Texture::isTextureRectangle() const {
#ifdef GL_TEXTURE_RECTANGLE_ARB
    return (type_ == GL_TEXTURE_RECTANGLE_ARB);
#else
    return false;
#endif
}

bool Texture::isDepthTexture() const {
    return internalformat_ == GL_DEPTH_COMPONENT
        || internalformat_ == GL_DEPTH_COMPONENT16 
        || internalformat_ == GL_DEPTH_COMPONENT24
        || internalformat_ == GL_DEPTH_COMPONENT32
        || internalformat_ == GL_DEPTH_COMPONENT32F;
}


} // namespace tgt
