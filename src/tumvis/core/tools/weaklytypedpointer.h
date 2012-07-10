#ifndef WEAKLYTYPEDPOINTER_H__
#define WEAKLYTYPEDPOINTER_H__

#include "tgt/assert.h"
#include "tgt/types.h"

namespace TUMVis {

    /**
     * Struct for handling void pointers slightly more typesafe.
     * Therfore it stores an enum value together with the pointer infering its data type.
     */
    struct WeaklyTypedPointer {
        /**
         * Base data type.
         **/
        enum PointerType {
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
        static size_t numBytes(PointerType pt) {
            switch (pt) {
                case WeaklyTypedPointer::UINT8:
                case WeaklyTypedPointer::INT8:
                    return 1;
                case WeaklyTypedPointer::UINT16:
                case WeaklyTypedPointer::INT16:
                    return 2;
                case WeaklyTypedPointer::UINT32:
                case WeaklyTypedPointer::INT32:
                    return sizeof(4);
                case WeaklyTypedPointer::FLOAT:
                    return sizeof(float);
                case WeaklyTypedPointer::UINT64:
                case WeaklyTypedPointer::INT64:
                    return sizeof(8);
                case WeaklyTypedPointer::DOUBLE:
                    return sizeof(double);
                default:
                    tgtAssert(false, "Should not reach this - called WeaklyTypedPointer::numBytes() with wrong argument!");
                    return 1;
            }
        };

        /**
         * Constructs a new weakly typed pointer.
         * \param pt    Base data type of the pointer.
         * \param ptr   Pointer to the data.
         */
        WeaklyTypedPointer(PointerType pt, void* ptr)
            : _pointerType(pt)
            , _pointer(ptr)
        {
        };

        /**
         * Returns the number of bytes occupied by one element of the type of this pointer.
         * \returns The number of bytes occupied by one element of the type of this pointer.
         */
        size_t numBytes() const {
            return WeaklyTypedPointer::numBytes(_pointerType);
        }

        PointerType _pointerType;   ///< Base data type of the pointer
        void* _pointer;             ///< Pointer to the data
    };


    /**
     * Some compile time traits for WeaklyTypedPointers.
     * \tparam  pt  Data type to check.
     */
    template<WeaklyTypedPointer::PointerType pt>
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
        static size_t numBytesLocal() { return 1; };
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
