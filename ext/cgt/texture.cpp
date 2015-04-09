/**********************************************************************
 *                                                                    *
 * cgt - CAMP Graphics Toolbox, Copyright (C) 2012-2014               *
 *     Chair for Computer Aided Medical Procedures                    *
 *     Technische Universitaet Muenchen, Germany.                     *
 *     <http://campar.in.tum.de/>                                     *
 *                                                                    *
 * forked from tgt - Tiny Graphics Toolbox, Copyright (C) 2006-2011   *
 *     Visualization and Computer Graphics Group, Department of       *
 *     Computer Science, University of Muenster, Germany.             *
 *     <http://viscg.uni-muenster.de>                                 *
 *                                                                    *
 * This file is part of the cgt library. This library is free         *
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

namespace cgt {

Texture::Texture(GLenum type, const cgt::ivec3& dimensions, GLint internalFormat, Filter filter /*= LINEAR*/) 
    : dimensions_(dimensions)
    , internalformat_(internalFormat)
    , filter_(filter)
    , wrapping_(CLAMP)
    , id_(0)
    , type_(type)
    , bpp_(0)
{
    init();
    uploadTexture(nullptr, calcMatchingFormat(internalFormat), calcMatchingDataType(internalFormat));
}

Texture::Texture(GLenum type, const cgt::ivec3& dimensions, GLint internalFormat, GLubyte* data, GLint format, GLenum dataType, Filter filter /*= LINEAR*/) 
    : dimensions_(dimensions)
    , internalformat_(internalFormat)
    , filter_(filter)
    , wrapping_(CLAMP)
    , id_(0)
    , type_(type)
    , bpp_(0)
{
    init();
    uploadTexture(data, format, dataType);
}


Texture::~Texture() {
    if (id_)
        GLGC.addGarbageTexture(id_);
}


void Texture::init() {
    bpp_ = calcBpp(internalformat_);
    generateId();
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
            LWARNINGC("cgt.Texture", "unknown dataType");
    }

    return typeSize * numChannels;
}

int Texture::calcBpp(GLint internalformat) {
    // supports all formats from https://www.opengl.org/wiki/GLAPI/glTexImage2D

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
        case GL_R16:
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
            cgtAssert(false, "Unknown internal format, this should not happen!");
            break;
    }

    return bpp;
}

int Texture::calcNumChannels(GLint internalFormat) {
    // supports all formats from https://www.opengl.org/wiki/GLAPI/glTexImage2D
    switch (internalFormat) {
        case 1:
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32:
        case GL_DEPTH_COMPONENT32F:
        case GL_RED:
        case GL_R8:
        case GL_R8_SNORM:
        case GL_R16:
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
            cgtAssert(false, "Unknown internal format, this should not happen!");
            return 0;
    }
}

int Texture::getSizeOnGPU() const {
    int bpp = calcBpp(internalformat_);
    return bpp * hmul(dimensions_);
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

    // set wrapping for all diminesions
    glTexParameteri(type_, GL_TEXTURE_WRAP_S, wrap);

    if (type_ == GL_TEXTURE_2D || type_ == GL_TEXTURE_2D_ARRAY || type_ == GL_TEXTURE_3D )
        glTexParameteri(type_, GL_TEXTURE_WRAP_T, wrap);
    if (GL_TEXTURE_3D)
        glTexParameteri(type_, GL_TEXTURE_WRAP_R, wrap);
}

void Texture::uploadTexture(const GLubyte* data, GLint format, GLenum dataType) {
    bind();

    switch (type_) {
        case GL_TEXTURE_1D:
            glTexImage1D(type_, 0, internalformat_, dimensions_.x, 0, format, dataType, data);
            if (filter_ == MIPMAP)
                glGenerateMipmap(GL_TEXTURE_1D);
            break;

        case GL_TEXTURE_1D_ARRAY: // fallthrough
        case GL_TEXTURE_2D:
            glTexImage2D(type_, 0, internalformat_, dimensions_.x, dimensions_.y, 0, format, dataType, data);
            if (filter_ == MIPMAP)
                glGenerateMipmap(GL_TEXTURE_2D);
            break;

        case GL_TEXTURE_2D_ARRAY: // fallthrough
        case GL_TEXTURE_3D:
            glTexImage3D(type_, 0, internalformat_, dimensions_.x, dimensions_.y, dimensions_.z, 0, format, dataType, data);
            if (filter_ == MIPMAP)
                glGenerateMipmap(GL_TEXTURE_3D);
            break;
    }

    LGL_ERROR;
}

GLubyte* Texture::downloadTextureToBuffer(GLint format, GLenum dataType) const {
    bind();

    int arraySize = hmul(dimensions_) * calcBpp(format, dataType);
    GLubyte* pixels = new GLubyte[arraySize];

    glPixelStorei(GL_PACK_ALIGNMENT, 1);
    glGetTexImage(type_, 0, format, dataType, pixels);
    return pixels;
}

bool Texture::isDepthTexture() const {
    return internalformat_ == GL_DEPTH_COMPONENT
        || internalformat_ == GL_DEPTH_COMPONENT16 
        || internalformat_ == GL_DEPTH_COMPONENT24
        || internalformat_ == GL_DEPTH_COMPONENT32
        || internalformat_ == GL_DEPTH_COMPONENT32F;
}

void Texture::setPriority(GLclampf p) {
    glPrioritizeTextures(1, &id_, &p);
}

bool Texture::isResident() const {
    GLboolean resident;
    return glAreTexturesResident(1, &id_, &resident) == GL_TRUE;
}

GLint Texture::calcInternalFormat(GLint format, GLenum dataType) {
    switch (format) {
        case GL_RED:
            switch (dataType) {
                case GL_UNSIGNED_BYTE:
                case GL_BYTE:
                    return GL_R8;
                case GL_UNSIGNED_SHORT:
                case GL_SHORT:
                    return GL_R16;
                case GL_UNSIGNED_INT:
                case GL_INT:
                    return GL_R32F;
                case GL_FLOAT:
                    return GL_R32F;
                default:
                    cgtAssert(false, "Should not reach this - wrong base data type!");
                    return GL_RED;
            }
        case GL_RG:
            switch (dataType) {
                case GL_UNSIGNED_BYTE:
                case GL_BYTE:
                    return GL_RG8;
                case GL_UNSIGNED_SHORT:
                case GL_SHORT:
                    return GL_RG16;
                case GL_UNSIGNED_INT:
                case GL_INT:
                    return GL_RG32F;
                case GL_FLOAT:
                    return GL_RG32F;
                default:
                    cgtAssert(false, "Should not reach this - wrong base data type!");
                    return GL_RG;
            }
        case GL_RGB:
            switch (dataType) {
                case GL_UNSIGNED_BYTE:
                case GL_BYTE:
                    return GL_RGB8;
                case GL_UNSIGNED_SHORT:
                case GL_SHORT:
                    return GL_RGB16;
                case GL_UNSIGNED_INT:
                case GL_INT:
                    return GL_RGB32F;
                case GL_FLOAT:
                    return GL_RGB32F;
                default:
                    cgtAssert(false, "Should not reach this - wrong base data type!");
                    return GL_RGB;
            }
        case GL_RGBA:
            switch (dataType) {
                case GL_UNSIGNED_BYTE:
                case GL_BYTE:
                    return GL_RGBA8;
                case GL_UNSIGNED_SHORT:
                case GL_SHORT:
                    return GL_RGBA16;
                case GL_UNSIGNED_INT:
                case GL_INT:
                    return GL_RGBA32F;
                case GL_FLOAT:
                    return GL_RGBA32F;
                default:
                    cgtAssert(false, "Should not reach this - wrong base data type!");
                    return GL_RGBA;
            }
        default:
            cgtAssert(false, "Should not reach this, wrong number of channels!");
            return GL_RED;
    }
}

GLint Texture::calcMatchingFormat(GLint internalFormat) {
    // supports all formats from http://www.opengl.org/sdk/docs/man/xhtml/glTexImage2D.xml
    switch (internalFormat) {
        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32:
        case GL_DEPTH_COMPONENT32F:
            return GL_DEPTH_COMPONENT;

        case GL_RED:
        case GL_R8:
        case GL_R8_SNORM:
        case GL_R16:
        case GL_R16_SNORM:
        case GL_R16F:
        case GL_R32F:
            return GL_RED;
            break;
        case GL_R8I:
        case GL_R8UI:
        case GL_R16I:
        case GL_R16UI:
        case GL_R32I:
        case GL_R32UI:
            return GL_RED_INTEGER;
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
            return GL_RG;
            break;
        case GL_RG8I:
        case GL_RG8UI:
        case GL_RG16I:
        case GL_RG16UI:
        case GL_RG32I:
        case GL_RG32UI:
            return GL_RG_INTEGER;
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
        case GL_RGB16:
        case GL_RGB16F:
        case GL_RGB32F:
        case GL_R11F_G11F_B10F:
        case GL_RGB9_E5:
            return GL_RGB;
            break;
        case GL_RGB8I:
        case GL_RGB8UI:
        case GL_RGB16I:
        case GL_RGB16UI:
        case GL_RGB32I:
        case GL_RGB32UI:
            return GL_RGB_INTEGER;
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
            return GL_RGBA;
            break;
        case GL_RGBA8I:
        case GL_RGBA8UI:
        case GL_RGBA16I:
        case GL_RGBA16UI:
        case GL_RGBA32I:
        case GL_RGBA32UI:
            return GL_RGBA_INTEGER;
            break;

        default:
            cgtAssert(false, "Unknown internal format, this should not happen!");
            return 0;
    }
}

GLenum Texture::calcMatchingDataType(GLint internalFormat) {
    // supports all formats from http://www.opengl.org/sdk/docs/man/xhtml/glTexImage2D.xml
    switch (internalFormat) {
        case GL_R8:
        case GL_R8UI:
        case GL_RG8:
        case GL_RG8UI:
        case GL_RGB8:
        case GL_RGB8UI:
        case GL_RGBA8:
        case GL_RGBA8UI:
        case GL_RGB4:
        case GL_SRGB8:
        case GL_RGBA2:
        case GL_RGBA4:
        case GL_SRGB8_ALPHA8:
            return GL_UNSIGNED_BYTE;
            break;


        case GL_R8_SNORM:
        case GL_R8I:
        case GL_RG8_SNORM:
        case GL_RG8I:
        case GL_RGB8_SNORM:
        case GL_RGB8I:
        case GL_RGBA8_SNORM:
        case GL_RGBA8I:
            return GL_BYTE;
            break;


        case GL_R16:
        case GL_R16UI:
        case GL_RG16:
        case GL_RG16UI:
        case GL_RGB16:
        case GL_RGB16UI:
        case GL_RGBA16:
        case GL_RGBA16UI:
            return GL_UNSIGNED_SHORT;
            break;


        case GL_R16_SNORM:
        case GL_R16I:
        case GL_RG16_SNORM:
        case GL_RG16I:
        case GL_RGB16_SNORM:
        case GL_RGB16I:
        case GL_RGBA16_SNORM:
        case GL_RGBA16I:
            return GL_SHORT;
            break;


        case GL_R32UI:
        case GL_RG32UI:
        case GL_RGB32UI:
        case GL_RGBA32UI:
            return GL_UNSIGNED_INT;
            break;


        case GL_R32I:
        case GL_RG32I:
        case GL_RGB32I:
        case GL_RGBA32I:
            return GL_INT;
            break;

        case GL_DEPTH_COMPONENT:
        case GL_DEPTH_COMPONENT16:
        case GL_DEPTH_COMPONENT24:
        case GL_DEPTH_COMPONENT32:
        case GL_DEPTH_COMPONENT32F:
        case GL_DEPTH_STENCIL:
        case GL_R16F:
        case GL_R32F:
        case GL_RG16F:
        case GL_RG32F:
        case GL_RGB16F:
        case GL_RGB32F:
        case GL_RGBA16F:
        case GL_RGBA32F:
        case GL_R3_G3_B2:
        case GL_RGB5:
        case GL_RGB10:
        case GL_RGB12:
        case GL_R11F_G11F_B10F:
        case GL_RGB9_E5:
        case GL_RGB5_A1:
        case GL_RGB10_A2:
        case GL_RGB10_A2UI:
        case GL_RGBA12:
            return GL_FLOAT;
            break;

        default:
            cgtAssert(false, "Unknown internal format, this should not happen!");
            return 0;
    }
}


} // namespace cgt
