#ifndef WEAKLYTYPEDPOINTER_H__
#define WEAKLYTYPEDPOINTER_H__

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/texture.h"
#include "tgt/tgt_gl.h"
#include "tgt/types.h"

namespace TUMVis {

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
            FLOAT,      ///< float
            UINT64,     ///< unsigned 64 bit integer
            INT64,      ///< signed 64 bit integer  q   
            DOUBLE      ///< double
        };

        /**
         * Returns the number of bytes occupied by one element of the type \a pt.
         * 
         * \note    There is a compiletime version in WeaklyTypedPointerTraits.
         * \sa      WeaklyTypedPointerTraits::numBytes()
         * \param   pt  Data type to check.
         * \returns The number of bytes occupied by one element of the type \a pt.
         */
        static size_t numBytes(BaseType pt, size_t numChannels);

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
         * \param ptr           Pointer to the data, WeaklyTypedPointer will take ownership of it.
         */
        WeaklyTypedPointer(BaseType pt, size_t numChannels, void* ptr);

        /**
         * Destructor, does _not_ delete the handled pointer!
         */
        virtual ~WeaklyTypedPointer();


        /**
         * Returns the number of bytes occupied by one element of the type of this pointer.
         * \returns The number of bytes occupied by one element of the type of this pointer.
         */
        size_t getNumBytesPerElement() const;

        GLint getGlFormat() const;

        GLenum getGlDataType() const;

        GLint getGlInternalFormat() const;

        BaseType _pointerType;      ///< Base data type of the pointer
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
         * \tparam  pt  Data type to check.
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

    template<>
    struct WeaklyTypedPointerTraits<WeaklyTypedPointer::UINT64> {
        static size_t numBytes() { return 8; };
    };

    template<>
    struct WeaklyTypedPointerTraits<WeaklyTypedPointer::INT64> {
        static size_t numBytes() { return 8; };
    };

    template<>
    struct WeaklyTypedPointerTraits<WeaklyTypedPointer::DOUBLE> {
        static size_t numBytes() { return sizeof(double); };
    };


}

#endif // WEAKLYTYPEDPOINTER_H__
