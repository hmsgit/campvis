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

#ifndef WEAKLYTYPEDPOINTER_H__
#define WEAKLYTYPEDPOINTER_H__

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/texture.h"
#include "tgt/tgt_gl.h"
#include "tgt/types.h"

#ifdef CAMPVIS_HAS_MODULE_DEVIL
#include <IL/il.h>
#include <IL/ilu.h>
#endif

namespace campvis {

    /**
     * Struct for handling void pointers slightly more typesafe.
     * Therfore it stores an enum value together with the pointer for deducing its data type.
     * \note    WeaklyTypedPointer takes _not_ take ownership of its pointer, hence it won't take care of deleting it!
     */
    struct WeaklyTypedPointer {
        /**
         * Base data type.
         **/
        enum BaseType {
            UINT8,      ///< unsigned 8 bit integer
            INT8,       ///< signed 8 bit integer
            UINT16,     ///< unsigned 16 bit integer
            INT16,      ///< signed 16 bit integer
            UINT32,     ///< unsigned 32 bit integer
            INT32,      ///< signed 32 bit integer
            FLOAT      ///< float
        };

        /**
         * Returns the number of bytes occupied by one element of the type \a bt with \a numChannels channels.
         * 
         * \note    There is a compiletime version in WeaklyTypedPointerTraits.
         * \sa      WeaklyTypedPointerTraits::numBytes()
         * \param   bt          Image base type
         * \param   numChannels Number of channels per element.
         * \returns The number of bytes occupied by one image element with \a numChannels channels of the type \a bt.
         */
        static size_t numBytes(BaseType bt, size_t numChannels = 1);

        /**
         * Calculates the number of channels for the given OpenGL format.
         * Kinda inverse function of getGlFormat().
         * \sa      WeaklyTypedPointer::getGlFormat()
         * \param   glFormat    OpenGL format
         * \return  Number of channels of the given OpenGL format.
         */
        static size_t numChannels(GLint glFormat);

        /**
         * Calculates the base type for the given OpenGL data type.
         * Kinda inverse function of getGlDataType().
         * \sa      WeaklyTypedPointer::getGlDataTypet()
         * \param   glDataType  OpenGL format
         * \return  Base type of the given OpenGL data type.
         */
        static WeaklyTypedPointer::BaseType baseType(GLenum glDataType);

// = Starting with the real stuff =================================================================

        /**
         * Constructs a new weakly typed pointer.
         * \param pt            Base data type of the pointer.
         * \param numChannels   Number of channels, must be in [1, 4]!
         * \param ptr           Pointer to the data, WeaklyTypedPointer will _not_ take ownership of it.
         */
        WeaklyTypedPointer(BaseType pt, size_t numChannels, void* ptr);

        /**
         * Empty default constructor, you have to initialize everything yourself.
         */
        WeaklyTypedPointer();

        /**
         * Destructor, does _not_ delete the handled pointer!
         */
        virtual ~WeaklyTypedPointer();

        bool operator==(const WeaklyTypedPointer& rhs) const;


        /**
         * Returns the number of bytes occupied by one element of the type of this pointer.
         * \returns The number of bytes occupied by one element of the type of this pointer.
         */
        size_t getNumBytesPerElement() const;

        GLint getGlFormat() const;

        GLenum getGlDataType() const;

        GLint getGlInternalFormat() const;

        bool isInteger() const;

        bool isSigned() const;

#ifdef CAMPVIS_HAS_MODULE_DEVIL
        ILenum getIlFormat() const;

        ILenum getIlDataType() const;
#endif


        BaseType _baseType;         ///< Base data type of the pointer
        size_t _numChannels;        ///< Number of channels, must be in [1, 4]!
        void* _pointer;             ///< Pointer to the data

    private:

        static const std::string loggerCat_;
    };

// = Some traits for WeaklyTypedPointers ==========================================================

    /**
     * Some compile time traits for WeaklyTypedPointers.
     * \tparam  pt  Data type to check.
     */
    template<WeaklyTypedPointer::BaseType pt>
    struct WeaklyTypedPointerTraits {
        /**
         * Returns the number of bytes occupied by one element of the given type.
         * 
         * \note    There is a runtime version in WeaklyTypedPointer.
         * \sa      WeaklyTypedPointer::numBytes()
         * \tparam  bt  Data type to check.
         * \returns The number of bytes occupied by one element of the given type.
         */
        static size_t numBytes() { return 0; };
    };

// - Template Specialization ----------------------------------------------------------------------

    template<>
    struct WeaklyTypedPointerTraits<WeaklyTypedPointer::UINT8> {
        static size_t numBytes() { return 1; };
    };

    template<>
    struct WeaklyTypedPointerTraits<WeaklyTypedPointer::INT8> {
        static size_t numBytes() { return 1; };
    };

    template<>
    struct WeaklyTypedPointerTraits<WeaklyTypedPointer::UINT16> {
        static size_t numBytes() { return 2; };
    };

    template<>
    struct WeaklyTypedPointerTraits<WeaklyTypedPointer::INT16> {
        static size_t numBytes() { return 2; };
    };

    template<>
    struct WeaklyTypedPointerTraits<WeaklyTypedPointer::UINT32> {
        static size_t numBytes() { return 4; };
    };

    template<>
    struct WeaklyTypedPointerTraits<WeaklyTypedPointer::INT32> {
        static size_t numBytes() { return 4; };
    };

    template<>
    struct WeaklyTypedPointerTraits<WeaklyTypedPointer::FLOAT> {
        static size_t numBytes() { return sizeof(float); };
    };

}

#endif // WEAKLYTYPEDPOINTER_H__
