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

#ifndef TYPETRAITS_H__
#define TYPETRAITS_H__

#include "tgt/tgt_gl.h"
#include "tgt/tgt_math.h"
#include "tgt/vector.h"
#include "core/tools/weaklytypedpointer.h"
#include <limits>


namespace campvis {

// anonymous namespace for helper traits
// irrelevant for the outside world...
namespace {

    /**
     * Helper Traits for TypeTraits varying only in the number of channels.
     * Template specializations offer the following constants/methods:
     *  - const GLint glFormat: OpenGL format to use
     */
    template<size_t NUMCHANNELS>
    struct TypeTraitsHelperPerChannel {};

    template<>
    struct TypeTraitsHelperPerChannel<1> {
        static const GLint glFormat = GL_ALPHA;
    };

    template<>
    struct TypeTraitsHelperPerChannel<2> {
        static const GLint glFormat = GL_LUMINANCE_ALPHA;
    };

    template<>
    struct TypeTraitsHelperPerChannel<3> {
        static const GLint glFormat = GL_RGB;
    };

    template<>
    struct TypeTraitsHelperPerChannel<4> {
        static const GLint glFormat = GL_RGBA;
    };

// ================================================================================================
// ================================================================================================

    /**
     * Helper Traits for TypeTraits varying in the base type and in the number of channels.
     * Template specializations offer the following constants/methods:
     *  - const GLint glInternalFormat: OpenGL internal format to use
     */
    template<typename BASETYPE, size_t NUMCHANNELS>
    struct TypeTraitsHelperPerBasetypePerChannel {};

// use nifty macros for easier template specialization
#define SPCIALIZE_TTIF(type,nc,internalFormat) \
    template<> \
    struct TypeTraitsHelperPerBasetypePerChannel<type, nc> { \
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

// ================================================================================================
// ================================================================================================

    /**
     * Helper Traits for TypeTraits varying only in the base type.
     * Template specializations offer the following constants/methods:
     *  - const GLenum glInternalFormat: OpenGL data type
     *  - const WeaklyTypedPointer::BaseType: WeaklyTypedPointer base type to use
     *  - const bool isSigned: flag whether this base type is signed
     *  - const bool isFloat: flag wheter this base type is floating point or integer
     */
    template<typename BASETYPE>
    struct TypeTraitsHelperPerBasetype {};

    template<>
    struct TypeTraitsHelperPerBasetype<uint8_t> {
        static const GLenum glDataType = GL_UNSIGNED_BYTE;
        static const WeaklyTypedPointer::BaseType weaklyTypedPointerBaseType = WeaklyTypedPointer::UINT8;
        static const bool isSigned = false;
        static const bool isFloat = false;
    };

    template<>
    struct TypeTraitsHelperPerBasetype<int8_t> {
        static const GLenum glDataType = GL_BYTE;
        static const WeaklyTypedPointer::BaseType weaklyTypedPointerBaseType = WeaklyTypedPointer::INT8;
        static const bool isSigned = true;
        static const bool isFloat = false;
    };

    template<>
    struct TypeTraitsHelperPerBasetype<uint16_t> {
        static const GLenum glDataType = GL_UNSIGNED_SHORT;
        static const WeaklyTypedPointer::BaseType weaklyTypedPointerBaseType = WeaklyTypedPointer::UINT16;
        static const bool isSigned = false;
        static const bool isFloat = false;
    };

    template<>
    struct TypeTraitsHelperPerBasetype<int16_t> {
        static const GLenum glDataType = GL_SHORT;
        static const WeaklyTypedPointer::BaseType weaklyTypedPointerBaseType = WeaklyTypedPointer::INT16;
        static const bool isSigned = true;
        static const bool isFloat = false;
    };

    template<>
    struct TypeTraitsHelperPerBasetype<uint32_t> {
        static const GLenum glDataType = GL_UNSIGNED_INT;
        static const WeaklyTypedPointer::BaseType weaklyTypedPointerBaseType = WeaklyTypedPointer::UINT32;
        static const bool isSigned = false;
        static const bool isFloat = false;
    };

    template<>
    struct TypeTraitsHelperPerBasetype<int32_t> {
        static const GLenum glDataType = GL_INT;
        static const WeaklyTypedPointer::BaseType weaklyTypedPointerBaseType = WeaklyTypedPointer::INT32;
        static const bool isSigned = true;
        static const bool isFloat = false;
    };

    template<>
    struct TypeTraitsHelperPerBasetype<float> {
        static const GLenum glDataType = GL_FLOAT;
        static const WeaklyTypedPointer::BaseType weaklyTypedPointerBaseType = WeaklyTypedPointer::FLOAT;
        static const bool isSigned = true;
        static const bool isFloat = true;
    };

// ================================================================================================
// ================================================================================================

    /**
     * Helper Traits for TypeTraits of generic base type varying only in the base type.
     * Template specializations offer the following constants/methods:
     *  - typedef ElementType: Typedef for the type of a single image element
     *  - static BASETYPE getChannel(element, channel): Method for accessing a specific channel of the given image element
     *  - static void getChannel(element, channel, value): Method setting a specific channel of the given image element to \a value
     */
    template<typename BASETYPE, size_t NUMCHANNELS>
    struct TypeTraitsHelperOfBasetypePerChannel {};

    template<typename BASETYPE>
    struct TypeTraitsHelperOfBasetypePerChannel<BASETYPE, 1> {
        typedef BASETYPE ElementType;

        static inline BASETYPE getChannel(const ElementType& element, size_t channel) {
            tgtAssert(channel == 0, "Channel out of bounds!");
            return element;
        }

        static inline void setChannel(ElementType& element, size_t channel, BASETYPE value) {
            tgtAssert(channel == 0, "Channel out of bounds!");
            element = value;
        }
    };

    template<typename BASETYPE>
    struct TypeTraitsHelperOfBasetypePerChannel<BASETYPE, 2> {
        typedef tgt::Vector2< BASETYPE > ElementType;

        static inline BASETYPE getChannel(const ElementType& element, size_t channel) {
            tgtAssert(channel >= 0 && channel <= 1, "Channel out of bounds!");
            return element[channel];
        }

        static inline void setChannel(ElementType& element, size_t channel, BASETYPE value) {
            tgtAssert(channel >= 0 && channel <= 1, "Channel out of bounds!");
            element[channel] = value;
        }
    };

    template<typename BASETYPE>
    struct TypeTraitsHelperOfBasetypePerChannel<BASETYPE, 3> {
        typedef tgt::Vector3< BASETYPE > ElementType;

        static inline BASETYPE getChannel(const ElementType& element, size_t channel) {
            tgtAssert(channel >= 0 && channel <= 2, "Channel out of bounds!");
            return element[channel];
        }

        static inline void setChannel(ElementType& element, size_t channel, BASETYPE value) {
            tgtAssert(channel >= 0 && channel <= 2, "Channel out of bounds!");
            element[channel] = value;
        }
    };

    template<typename BASETYPE>
    struct TypeTraitsHelperOfBasetypePerChannel<BASETYPE, 4> {
        typedef tgt::Vector4< BASETYPE > ElementType;

        static inline BASETYPE getChannel(const ElementType& element, size_t channel) {
            tgtAssert(channel >= 0 && channel <= 3, "Channel out of bounds!");
            return element[channel];
        }

        static inline void setChannel(ElementType& element, size_t channel, BASETYPE value) {
            tgtAssert(channel >= 0 && channel <= 3, "Channel out of bounds!");
            element[channel] = value;
        }
    };

// ================================================================================================
// ================================================================================================
    
    /**
     * Helper Traits for TypeNormalizer.
     * Template specializations offer the following methods:
     *  - static float normalizeToFloat<T>(T value): Normalize the given value to float
     *  - static T denormalizeFromFloat<T>(float value): Denormalize the given float to T
     *
     * \note    Template parameter T of trait functions have to match trait template parameter flags!
     *
     * \tparam  ISFLOAT     Flag whether base type is floating point or integer
     * \tparam  ISSIGNED    Flag whether base type is signed or not (ignored when ISFLOAT == true)
     */
    template<typename T, bool ISFLOAT, bool ISSIGNED>
    struct TypeNormalizerHelper {};

    /**
     * Template specialization for floating point types - no normalization needed.
     */
    template<typename T, bool ISSIGNED>
    struct TypeNormalizerHelper<T, true, ISSIGNED> {
        static float normalizeToFloat(T value) {
            return static_cast<float>(value);
        }

        static T denormalizeFromFloat(float value) {
            return static_cast<T>(value);
        };
    };

    /**
     * Template specialization for unsigned integer types, map from [0, max] to [0.0, 1.0]
     */
    template<typename T>
    struct TypeNormalizerHelper<T, false, false> {
        static float normalizeToFloat(T value) {
            return static_cast<float>(value) / std::numeric_limits<T>::max();
        }

        static T denormalizeFromFloat(float value) {
            value = tgt::clamp(value, 0.0f, 1.0f);
            return static_cast<T>(value * std::numeric_limits<T>::max());
        };
    };

    /**
     * Template specialization for unsigned integer types, map from [min, max] to [0.0, 1.0]
     */
    template<typename T>
    struct TypeNormalizerHelper<T, false, true> {
        static float normalizeToFloat(T value) {
            if (value >= 0)
                return (static_cast<float>(value) / std::numeric_limits<T>::max()) * .5f + .5f;
            else
                return (static_cast<float>(value) / -std::numeric_limits<T>::min()) * .5f + .5f;
        }

        static T denormalizeFromFloat(float value) {
            value = (tgt::clamp(value, 0.0f, 1.0f) - .5f) * 2.f;
            if(value >= 0.0f)
                return static_cast<T>(value * std::numeric_limits<T>::max());
            else
                return static_cast<T>(value * -std::numeric_limits<T>::min());
        };
    };
}

// ================================================================================================
// = Starting with the actual stuff to use ======================================================== 
// ================================================================================================
    /**
     * Type traits for image data depending on base type and number of channels.
     *
     * \tparam  BASETYPE    Base type of the image data (type of a single channel of an image element)
     * \tparam  NUMCHANNELS Number of channels of the image data.
     **/
    template<typename BASETYPE, size_t NUMCHANNELS>
    struct TypeTraits {
        /// C++ type of one single image element.
        typedef typename TypeTraitsHelperOfBasetypePerChannel<BASETYPE, NUMCHANNELS>::ElementType ElementType;

        /// Size of the element base type in bytes.
        static const size_t basetypeSize = sizeof(BASETYPE);

        /// Number of channels of one single image element.
        static const size_t numChannels = NUMCHANNELS;

        /// Size of one single image element in bytes.
        static const size_t elementSize = sizeof(BASETYPE) * NUMCHANNELS;

        /// OpenGL format to apply for this image data.
        static const GLint glFormat = TypeTraitsHelperPerChannel<NUMCHANNELS>::glFormat;

        /// OpenGL data type to apply for this image data.
        static const GLenum glDataType = TypeTraitsHelperPerBasetype<BASETYPE>::glDataType;

        /// OpenGL internal format to apply for this image data.
        static const GLint glInternalFormat = TypeTraitsHelperPerBasetypePerChannel<BASETYPE, NUMCHANNELS>::glInteralFormat;

        /// WeaklyTypedPointer::BaseType for this image data.
        static const WeaklyTypedPointer::BaseType weaklyTypedPointerBaseType = TypeTraitsHelperPerBasetype<BASETYPE>::weaklyTypedPointerBaseType;

        /**
         * Returns the i-th channel (0-based) of the given image element \a element.
         * \param   element Image element to access
         * \param   channel Channel to access, must be within [0, NUMCHANNELS).
         * \return  The i-th channel of \a element.
         */
        static inline BASETYPE getChannel(const typename TypeTraitsHelperOfBasetypePerChannel<BASETYPE, NUMCHANNELS>::ElementType& element, size_t channel) {
            return TypeTraitsHelperOfBasetypePerChannel<BASETYPE, NUMCHANNELS>::getChannel(element, channel);
        };

        /**
         * Sets the i-th channel (0-based) of the given image element \a element.
         * \param   element Image Element to set.
         * \param   channel Channel to access, must be within [0, NUMCHANNELS).
         * \param   value   Value to set
         */
        static inline void setChannel(typename TypeTraitsHelperOfBasetypePerChannel<BASETYPE, NUMCHANNELS>::ElementType& element, size_t channel, BASETYPE value) {
            TypeTraitsHelperOfBasetypePerChannel<BASETYPE, NUMCHANNELS>::setChannel(element, channel, value);
        }

    };

    /**
     * Offers methods for (de)normalizing image element base types to/from float.
     * \note    Due to the use of templates and traits, the conversion function is completely
     *          determined at compile-time.
     */
    struct TypeNormalizer {
        /**
         * Normalizes the given value \a value to a float representation.
         *  - for \em unsigned integer types, the value range is mapped linearly to [0.0;1.0]
         *  - for \em signed integer types, the value range is mapped linearly to [-1.0;1.0]
         *  - floating point types are not mapped
         *
         * \tparam  T       Base type of the image element to normalize
         * \param   value   Value to normalize
         * \return  A normalized float representation of \a value.
         */
        template<typename T>
        static float normalizeToFloat(T value) {
            return TypeNormalizerHelper<T, TypeTraitsHelperPerBasetype<T>::isFloat, TypeTraitsHelperPerBasetype<T>::isSigned>::normalizeToFloat(value);
        };

        /**
        * Denormalizes the given value \a value from a float representation.
        *  - for \em unsigned integer types, the value range is mapped linearly to [0.0;1.0]
        *  - for \em signed integer types, the value range is mapped linearly to [-1.0;1.0]
        *  - floating point types are not mapped
        *
        * \tparam  T       Base type of the image element to denormalize
        * \param   value   Float representation to denormalize
        * \return  The denormalized value of the float representation \a value.
         */
        template<typename T>
        static T denormalizeFromFloat(float value) {
            return TypeNormalizerHelper<T, TypeTraitsHelperPerBasetype<T>::isFloat, TypeTraitsHelperPerBasetype<T>::isSigned>::denormalizeFromFloat(value);
        }
    };
}

#endif // TYPETRAITS_H__
