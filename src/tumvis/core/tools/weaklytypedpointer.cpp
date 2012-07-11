#include "weaklytypedpointer.h"

namespace TUMVis {
    const std::string WeaklyTypedPointer::loggerCat_ = "TUMVis.core.tools.WeaklyTypedPointer";

    WeaklyTypedPointer::WeaklyTypedPointer(BaseType pt, size_t numChannels, void* ptr)
        : _pointerType(pt)
        , _numChannels(numChannels)
        , _pointer(ptr)
    {
        tgtAssert(_numChannels > 0 && _numChannels <= 4, "Number of channels out of bounds!");
    };

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
                case WeaklyTypedPointer::UINT64:
                case WeaklyTypedPointer::INT64:
                    return 8 * numChannels;
                case WeaklyTypedPointer::DOUBLE:
                    return sizeof(double) * numChannels;
                default:
                    tgtAssert(false, "Should not reach this - called WeaklyTypedPointer::numBytes() with wrong argument!");
                    return 1;
        }
    };

    size_t WeaklyTypedPointer::getNumBytesPerElement() const {
        return WeaklyTypedPointer::numBytes(_pointerType, _numChannels);
    }

    GLint WeaklyTypedPointer::getGlFormat() const {
        switch (_numChannels) {
            case 1: 
                return GL_ALPHA;
            case 2: 
                return GL_LUMINANCE_ALPHA;
            case 3:
                return GL_RGB;
            case 4:
                return GL_RGBA;
            default:
                tgtAssert(false, "Should not reach hier, wrong number of channels!");
                return GL_ALPHA;
        }
    }

    GLenum WeaklyTypedPointer::getGlDataType() const  {
        switch (_pointerType) {
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
            case WeaklyTypedPointer::UINT64:
            case WeaklyTypedPointer::INT64:
            case WeaklyTypedPointer::DOUBLE:
                tgtAssert(false, "Base data type unsupported by OpenGL!");
                return GL_BYTE;
            default:
                tgtAssert(false, "Should not reach this - wrong base data type!");
                return GL_BYTE;
        }
    }
    GLint WeaklyTypedPointer::getGlInternalFormat() const{
        switch (_numChannels) {
            case 1:
                switch (_pointerType) {
                    case WeaklyTypedPointer::UINT8:
                    case WeaklyTypedPointer::INT8:
                        return GL_ALPHA8;
                    case WeaklyTypedPointer::UINT16:
                    case WeaklyTypedPointer::INT16:
                        return GL_ALPHA16;
                    case WeaklyTypedPointer::UINT32:
                    case WeaklyTypedPointer::INT32:
                        return GL_ALPHA;
                    case WeaklyTypedPointer::FLOAT:
                        return GL_ALPHA32F_ARB;
                    case WeaklyTypedPointer::UINT64:
                    case WeaklyTypedPointer::INT64:
                    case WeaklyTypedPointer::DOUBLE:
                        tgtAssert(false, "Base data type unsupported by OpenGL!");
                        return GL_BYTE;
                    default:
                        tgtAssert(false, "Should not reach this - wrong base data type!");
                        return GL_BYTE;
                }
            case 2:
                return GL_LUMINANCE_ALPHA;
            case 3:
                switch (_pointerType) {
                    case WeaklyTypedPointer::UINT8:
                    case WeaklyTypedPointer::INT8:
                        return GL_RGB8;
                    case WeaklyTypedPointer::UINT16:
                    case WeaklyTypedPointer::INT16:
                        return GL_RGB16;
                    case WeaklyTypedPointer::UINT32:
                    case WeaklyTypedPointer::INT32:
                        return GL_RGB;
                    case WeaklyTypedPointer::FLOAT:
                        return GL_RGB32F_ARB;
                    case WeaklyTypedPointer::UINT64:
                    case WeaklyTypedPointer::INT64:
                    case WeaklyTypedPointer::DOUBLE:
                        tgtAssert(false, "Base data type unsupported by OpenGL!");
                        return GL_BYTE;
                    default:
                        tgtAssert(false, "Should not reach this - wrong base data type!");
                        return GL_BYTE;
                }
            case 4:
                switch (_pointerType) {
                    case WeaklyTypedPointer::UINT8:
                    case WeaklyTypedPointer::INT8:
                        return GL_RGBA8;
                    case WeaklyTypedPointer::UINT16:
                    case WeaklyTypedPointer::INT16:
                        return GL_RGBA16;
                    case WeaklyTypedPointer::UINT32:
                    case WeaklyTypedPointer::INT32:
                        return GL_RGBA;
                    case WeaklyTypedPointer::FLOAT:
                        return GL_RGBA32F_ARB;
                    case WeaklyTypedPointer::UINT64:
                    case WeaklyTypedPointer::INT64:
                    case WeaklyTypedPointer::DOUBLE:
                        tgtAssert(false, "Base data type unsupported by OpenGL!");
                        return GL_BYTE;
                    default:
                        tgtAssert(false, "Should not reach this - wrong base data type!");
                        return GL_BYTE;
                }
            default:
                tgtAssert(false, "Should not reach hier, wrong number of channels!");
                return GL_ALPHA;
        }
    }

    size_t WeaklyTypedPointer::numChannels(GLint glFormat) {
        switch (glFormat) {
            case 1:
            case GL_COLOR_INDEX:
            case GL_RED:
            case GL_GREEN:
            case GL_BLUE:
            case GL_ALPHA:
            case GL_INTENSITY:
            case GL_LUMINANCE:
            case GL_DEPTH_COMPONENT:
            case GL_DEPTH_COMPONENT24:
            case GL_ALPHA_INTEGER_EXT:
                return 1;

            case 2:
            case GL_LUMINANCE_ALPHA:
                return 2;

            case 3:
            case GL_RGB:
            case GL_BGR:
                return 3;

            case 4:
            case GL_RGBA:
            case GL_BGRA:
            case GL_RGBA16:
            case GL_RGBA16F_ARB:
                return 4;

            default:
                tgtAssert(false, "Unsupported OpenGL data format.");
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
}
