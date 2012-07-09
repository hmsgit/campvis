#ifndef WEAKLYTYPEDPOINTER_H__
#define WEAKLYTYPEDPOINTER_H__

#include "tgt/assert.h"

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
            UCHAR,      ///< unsigned char
            CHAR,       ///< char
            USHORT,     ///< unsigned short
            SHORT,      ///< short
            UINT,       ///< unsigned int
            INT,        ///< int
            FLOAT,      ///< float
            ULONG,      ///< unsigned long
            LONG,       ///< long
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
                case WeaklyTypedPointer::UCHAR:
                    return sizeof(unsigned char);
                case WeaklyTypedPointer::CHAR:
                    return sizeof(char);
                case WeaklyTypedPointer::USHORT:
                    return sizeof(unsigned short);
                case WeaklyTypedPointer::SHORT:
                    return sizeof(short);
                case WeaklyTypedPointer::UINT:
                    return sizeof(unsigned int);
                case WeaklyTypedPointer::INT:
                    return sizeof(int);
                case WeaklyTypedPointer::FLOAT:
                    return sizeof(float);
                case WeaklyTypedPointer::ULONG:
                    return sizeof(unsigned long);
                case WeaklyTypedPointer::LONG:
                    return sizeof(long);
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
    struct WeaklyTypedPointerTraits<WeaklyTypedPointer::UCHAR> {
        static size_t numBytesLocal() { return sizeof(unsigned char); };
    };

    template<>
    struct WeaklyTypedPointerTraits<WeaklyTypedPointer::CHAR> {
        static size_t numBytes() { return sizeof(char); };
    };

    template<>
    struct WeaklyTypedPointerTraits<WeaklyTypedPointer::USHORT> {
        static size_t numBytes() { return sizeof(unsigned short); };
    };

    template<>
    struct WeaklyTypedPointerTraits<WeaklyTypedPointer::SHORT> {
        static size_t numBytes() { return sizeof(short); };
    };

    template<>
    struct WeaklyTypedPointerTraits<WeaklyTypedPointer::UINT> {
        static size_t numBytes() { return sizeof(unsigned int); };
    };

    template<>
    struct WeaklyTypedPointerTraits<WeaklyTypedPointer::INT> {
        static size_t numBytes() { return sizeof(int); };
    };

    template<>
    struct WeaklyTypedPointerTraits<WeaklyTypedPointer::FLOAT> {
        static size_t numBytes() { return sizeof(float); };
    };

    template<>
    struct WeaklyTypedPointerTraits<WeaklyTypedPointer::ULONG> {
        static size_t numBytes() { return sizeof(unsigned long); };
    };

    template<>
    struct WeaklyTypedPointerTraits<WeaklyTypedPointer::LONG> {
        static size_t numBytes() { return sizeof(long); };
    };

    template<>
    struct WeaklyTypedPointerTraits<WeaklyTypedPointer::DOUBLE> {
        static size_t numBytes() { return sizeof(double); };
    };


}

#endif // WEAKLYTYPEDPOINTER_H__
