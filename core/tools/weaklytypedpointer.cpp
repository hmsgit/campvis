// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#include "weaklytypedpointer.h"

namespace campvis {
    const std::string WeaklyTypedPointer::loggerCat_ = "CAMPVis.core.tools.WeaklyTypedPointer";

    WeaklyTypedPointer::WeaklyTypedPointer(BaseType pt, size_t numChannels, void* ptr)
        : _baseType(pt)
        , _numChannels(numChannels)
        , _pointer(ptr)
    {
        tgtAssert(_numChannels > 0 && _numChannels <= 4, "Number of channels out of bounds!");
    };

    WeaklyTypedPointer::WeaklyTypedPointer()
        : _baseType(UINT8)
        , _numChannels(0)
        , _pointer(0)
    {
    }

    WeaklyTypedPointer::~WeaklyTypedPointer() {
        // We do _not_ own the pointer, so we don't need to delete it.
    }

    size_t WeaklyTypedPointer::numBytes(BaseType pt, size_t numChannels) {
        switch (pt) {
            case WeaklyTypedPointer::UINT8:
            case WeaklyTypedPointer::INT8:
                return 1 * numChannels;
            case WeaklyTypedPointer::UINT16:
            case WeaklyTypedPointer::INT16:
                return 2 * numChannels;
            case WeaklyTypedPointer::UINT32:
            case WeaklyTypedPointer::INT32:
                return 4 * numChannels;
            case WeaklyTypedPointer::FLOAT:
                return sizeof(float) * numChannels;
            default:
                tgtAssert(false, "Should not reach this - called WeaklyTypedPointer::numBytes() with wrong argument!");
                return 1;
        }
    };

    size_t WeaklyTypedPointer::getNumBytesPerElement() const {
        return WeaklyTypedPointer::numBytes(_baseType, _numChannels);
    }

    GLint WeaklyTypedPointer::getGlFormat() const {
        switch (_numChannels) {
            case 1: 
                return GL_RED;
            case 2: 
                return GL_RG;
            case 3:
                return GL_RGB;
            case 4:
                return GL_RGBA;
            default:
                tgtAssert(false, "Should not reach this, wrong number of channels!");
                return GL_RED;
        }
    }

    GLenum WeaklyTypedPointer::getGlDataType() const  {
        switch (_baseType) {
            case WeaklyTypedPointer::UINT8:
                return GL_UNSIGNED_BYTE;
            case WeaklyTypedPointer::INT8:
                return GL_BYTE;
            case WeaklyTypedPointer::UINT16:
                return GL_UNSIGNED_SHORT;
            case WeaklyTypedPointer::INT16:
                return GL_SHORT;
            case WeaklyTypedPointer::UINT32:
                return GL_UNSIGNED_INT;
            case WeaklyTypedPointer::INT32:
                return GL_INT;
            case WeaklyTypedPointer::FLOAT:
                return GL_FLOAT;
            default:
                tgtAssert(false, "Should not reach this - wrong base data type!");
                return GL_BYTE;
        }
    }
    GLint WeaklyTypedPointer::getGlInternalFormat() const{
        switch (_numChannels) {
            case 1:
                switch (_baseType) {
                    case WeaklyTypedPointer::UINT8:
                    case WeaklyTypedPointer::INT8:
                        return GL_R8;
                    case WeaklyTypedPointer::UINT16:
                    case WeaklyTypedPointer::INT16:
                        return GL_R16;
                    case WeaklyTypedPointer::UINT32:
                    case WeaklyTypedPointer::INT32:
                        return GL_R32F;
                    case WeaklyTypedPointer::FLOAT:
                        return GL_R32F;
                    default:
                        tgtAssert(false, "Should not reach this - wrong base data type!");
                        return GL_RED;
                }
            case 2:
                switch (_baseType) {
                    case WeaklyTypedPointer::UINT8:
                    case WeaklyTypedPointer::INT8:
                        return GL_RG8;
                    case WeaklyTypedPointer::UINT16:
                    case WeaklyTypedPointer::INT16:
                        return GL_RG16;
                    case WeaklyTypedPointer::UINT32:
                    case WeaklyTypedPointer::INT32:
                        return GL_RG32F;
                    case WeaklyTypedPointer::FLOAT:
                        return GL_RG32F;
                    default:
                        tgtAssert(false, "Should not reach this - wrong base data type!");
                        return GL_RG;
                }
            case 3:
                switch (_baseType) {
                    case WeaklyTypedPointer::UINT8:
                    case WeaklyTypedPointer::INT8:
                        return GL_RGB8;
                    case WeaklyTypedPointer::UINT16:
                    case WeaklyTypedPointer::INT16:
                        return GL_RGB16;
                    case WeaklyTypedPointer::UINT32:
                    case WeaklyTypedPointer::INT32:
                        return GL_RGB32F;
                    case WeaklyTypedPointer::FLOAT:
                        return GL_RGB32F;
                    default:
                        tgtAssert(false, "Should not reach this - wrong base data type!");
                        return GL_RGB;
                }
            case 4:
                switch (_baseType) {
                    case WeaklyTypedPointer::UINT8:
                    case WeaklyTypedPointer::INT8:
                        return GL_RGBA8;
                    case WeaklyTypedPointer::UINT16:
                    case WeaklyTypedPointer::INT16:
                        return GL_RGBA16;
                    case WeaklyTypedPointer::UINT32:
                    case WeaklyTypedPointer::INT32:
                        return GL_RGBA32F;
                    case WeaklyTypedPointer::FLOAT:
                        return GL_RGBA32F;
                    default:
                        tgtAssert(false, "Should not reach this - wrong base data type!");
                        return GL_RGBA;
                }
            default:
                tgtAssert(false, "Should not reach hier, wrong number of channels!");
                return GL_RED;
        }
    }

    size_t WeaklyTypedPointer::numChannels(GLint glFormat) {
        // supports all formats from http://www.opengl.org/sdk/docs/man/xhtml/glTexImage2D.xml
        switch (glFormat) {
            case 1:
            case GL_DEPTH_COMPONENT:
            case GL_DEPTH_COMPONENT16:
            case GL_DEPTH_COMPONENT24:
            case GL_DEPTH_COMPONENT32:
            case GL_DEPTH_COMPONENT32F:
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
                return 4;
                break;

            default:
                tgtAssert(false, "Should not reach this, wrong number of gl format!");
                return 0;
        }
    }

    WeaklyTypedPointer::BaseType WeaklyTypedPointer::baseType(GLenum glDataType) {
        switch (glDataType) {
            case GL_UNSIGNED_BYTE:
                return WeaklyTypedPointer::UINT8;
            case GL_BYTE:
                return WeaklyTypedPointer::INT8;
            case GL_UNSIGNED_SHORT:
                return WeaklyTypedPointer::UINT16;
            case GL_SHORT:
                return WeaklyTypedPointer::INT16;
            case GL_UNSIGNED_INT:
                return WeaklyTypedPointer::UINT32;
            case GL_INT:
                return WeaklyTypedPointer::INT32;
            case GL_FLOAT:
                return WeaklyTypedPointer::FLOAT;
            default:
                tgtAssert(false, "Unsupported OpenGL data type.");
                return WeaklyTypedPointer::INT8;
        }
    }

    bool WeaklyTypedPointer::operator==(const WeaklyTypedPointer& rhs) const {
        return (_baseType == rhs._baseType) && (_numChannels == rhs._numChannels) && (_pointer == rhs._pointer);
    }

    bool WeaklyTypedPointer::isInteger() const {
        return (_baseType != FLOAT);
    }

    bool WeaklyTypedPointer::isSigned() const {
        return (_baseType == INT8) || (_baseType == INT16) || (_baseType == INT32);
    }

#ifdef HAS_KISSCL
    cl_channel_type WeaklyTypedPointer::getClChannelType() const {
        switch (_baseType) {
            case WeaklyTypedPointer::UINT8:
                return CL_UNORM_INT8;
            case WeaklyTypedPointer::INT8:
                return CL_SNORM_INT8;
            case WeaklyTypedPointer::UINT16:
                return CL_UNORM_INT16;
            case WeaklyTypedPointer::INT16:
                return CL_SNORM_INT16;
            case WeaklyTypedPointer::UINT32:
                return CL_UNSIGNED_INT32;
            case WeaklyTypedPointer::INT32:
                return CL_SIGNED_INT32;
            case WeaklyTypedPointer::FLOAT:
                return CL_FLOAT;
            default:
                tgtAssert(false, "Should not reach this - wrong base data type!");
                return CL_SIGNED_INT8;
        }
    }

    cl_channel_order WeaklyTypedPointer::getClChannelOrder() const {
        switch (_numChannels) {
            case 1: 
                return CL_INTENSITY;
            case 2: 
                return CL_RA;
            case 4:
                return CL_RGBA;
            default:
                LERROR("Unsupported number of channels.");
                return CL_A;
        }
    }
#endif

#ifdef CAMPVIS_HAS_MODULE_DEVIL
    ILenum WeaklyTypedPointer::getIlFormat() const {
        switch (_numChannels) {
            case 1: 
                return IL_ALPHA;
            case 2: 
                return IL_LUMINANCE_ALPHA;
            case 3:
                return IL_RGB;
            case 4:
                return IL_RGBA;
            default:
                tgtAssert(false, "Should not reach this, wrong number of channels!");
                return IL_ALPHA;
        }
    }

    ILenum WeaklyTypedPointer::getIlDataType() const {
        switch (_baseType) {
            case WeaklyTypedPointer::UINT8:
                return IL_UNSIGNED_BYTE;
            case WeaklyTypedPointer::INT8:
                return IL_BYTE;
            case WeaklyTypedPointer::UINT16:
                return IL_UNSIGNED_SHORT;
            case WeaklyTypedPointer::INT16:
                return IL_SHORT;
            case WeaklyTypedPointer::UINT32:
                return IL_UNSIGNED_INT;
            case WeaklyTypedPointer::INT32:
                return IL_INT;
            case WeaklyTypedPointer::FLOAT:
                return IL_FLOAT;
            default:
                tgtAssert(false, "Should not reach this - wrong base data type!");
                return GL_BYTE;
        }
    }
#endif

}
