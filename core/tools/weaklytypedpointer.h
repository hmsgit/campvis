// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#ifndef WEAKLYTYPEDPOINTER_H__
#define WEAKLYTYPEDPOINTER_H__

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/texture.h"
#include "tgt/tgt_gl.h"
#include "tgt/types.h"

#include "kisscl/kisscl.h"

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

        cl_channel_type getClChannelType() const;

        cl_channel_order getClChannelOrder() const;

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
