#ifndef typetraits_h__
#define typetraits_h__

#include "tgt/tgt_gl.h"
#include "core/datastructures/imagedataram.h"
#include <limits>

namespace TUMVis {

// = per-channel traits =========================================================================== 

    template<size_t NUMCHANNELS>
    struct TypeTraitsNumChannels {
    };

    template<>
    struct TypeTraitsNumChannels<1> {
        static const GLint glFormat = GL_ALPHA;
    };

    template<>
    struct TypeTraitsNumChannels<2> {
        static const GLint glFormat = GL_LUMINANCE_ALPHA;
    };

    template<>
    struct TypeTraitsNumChannels<3> {
        static const GLint glFormat = GL_RGB;
    };

    template<>
    struct TypeTraitsNumChannels<4> {
        static const GLint glFormat = GL_RGBA;
    };

// = per-channel traits =========================================================================== 

    template<typename BASETYPE>
    struct TypeTraitsType {
    };

    template<>
    struct TypeTraitsType<uint8_t> {
        static const GLenum glDataType = GL_UNSIGNED_BYTE;
    };

    template<>
    struct TypeTraitsType<int8_t> {
        static const GLenum glDataType = GL_BYTE;
    };

    template<>
    struct TypeTraitsType<uint16_t> {
        static const GLenum glDataType = GL_UNSIGNED_SHORT;
    };

    template<>
    struct TypeTraitsType<int16_t> {
        static const GLenum glDataType = GL_SHORT;
    };

    template<>
    struct TypeTraitsType<uint32_t> {
        static const GLenum glDataType = GL_UNSIGNED_INT;
    };

    template<>
    struct TypeTraitsType<int32_t> {
        static const GLenum glDataType = GL_INT;
    };

    template<>
    struct TypeTraitsType<float> {
        static const GLenum glDataType = GL_FLOAT;
    };

// = per-channel traits =========================================================================== 

    template<typename BASETYPE, size_t NUMCHANNELS>
    struct TypeTraitsInternalFormat {
    };

#define SPCIALIZE_TTIF(type,nc,internalFormat) \
    template<> \
    struct TypeTraitsInternalFormat<type, nc> { \
        static const GLint glInteralFormat = internalFormat; \
    }; \

    SPCIALIZE_TTIF(uint8_t, 1, GL_ALPHA8)
    SPCIALIZE_TTIF(int8_t,  1, GL_ALPHA8)
    SPCIALIZE_TTIF(uint16_t,1, GL_ALPHA16)
    SPCIALIZE_TTIF(int16_t, 1, GL_ALPHA16)
    SPCIALIZE_TTIF(uint32_t,1, GL_ALPHA)
    SPCIALIZE_TTIF(int32_t, 1, GL_ALPHA)
    SPCIALIZE_TTIF(float,   1, GL_ALPHA32F_ARB)

    SPCIALIZE_TTIF(uint8_t, 2, GL_LUMINANCE_ALPHA)
    SPCIALIZE_TTIF(int8_t,  2, GL_LUMINANCE_ALPHA)
    SPCIALIZE_TTIF(uint16_t,2, GL_LUMINANCE_ALPHA)
    SPCIALIZE_TTIF(int16_t, 2, GL_LUMINANCE_ALPHA)
    SPCIALIZE_TTIF(uint32_t,2, GL_LUMINANCE_ALPHA)
    SPCIALIZE_TTIF(int32_t, 2, GL_LUMINANCE_ALPHA)
    SPCIALIZE_TTIF(float,   2, GL_LUMINANCE_ALPHA)

    SPCIALIZE_TTIF(uint8_t, 3, GL_RGB8)
    SPCIALIZE_TTIF(int8_t,  3, GL_RGB8)
    SPCIALIZE_TTIF(uint16_t,3, GL_RGB16)
    SPCIALIZE_TTIF(int16_t, 3, GL_RGB16)
    SPCIALIZE_TTIF(uint32_t,3, GL_RGB)
    SPCIALIZE_TTIF(int32_t, 3, GL_RGB)
    SPCIALIZE_TTIF(float,   3, GL_RGB32F_ARB)

    SPCIALIZE_TTIF(uint8_t, 4, GL_RGBA8)
    SPCIALIZE_TTIF(int8_t,  4, GL_RGBA8)
    SPCIALIZE_TTIF(uint16_t,4, GL_RGBA16)
    SPCIALIZE_TTIF(int16_t, 4, GL_RGBA16)
    SPCIALIZE_TTIF(uint32_t,4, GL_RGBA)
    SPCIALIZE_TTIF(int32_t, 4, GL_RGBA)
    SPCIALIZE_TTIF(float,   4, GL_RGBA32F_ARB)

// = per-channel traits =========================================================================== 

    /**
     * 
     **/
    template<typename BASETYPE, size_t NUMCHANNELS>
    struct TypeTraits {
        typedef typename ImageDataRAMTraits<BASETYPE, NUMCHANNELS>::ImageType ImageRAMType;

        /**
         * Returns the size of the element base type in bytes.
         */
        static const size_t basetypeSize = sizeof(BASETYPE);

        /**
         * Returns the number of channels per image element.
         */
        static const size_t numChannels = NUMCHANNELS;

        /**
         * Returns the number of bytes required for one image element.
         */
        static const size_t elementSize = sizeof(BASETYPE) * NUMCHANNELS;

        static const GLint glFormat = TypeTraitsNumChannels<NUMCHANNELS>::glFormat;
        static const GLenum glDataType = TypeTraitsType<BASETYPE>::glDataType;
        static const GLint glInternalFormat = TypeTraitsInternalFormat<BASETYPE, NUMCHANNELS>::glInteralFormat;
    };

// - Template specializations ---------------------------------------------------------------------

}


#endif