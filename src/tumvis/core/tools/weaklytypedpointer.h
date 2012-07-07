#ifndef WEAKLYTYPEDPOINTER_H__
#define WEAKLYTYPEDPOINTER_H__

#include "tgt/assert.h"

namespace TUMVis {

    /**
     * Struct for handling void pointers slightly more typesafe.
     * Therfore it stores an enum value together with the pointer infering its data type.
     **/
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
         * \param   pt  Data type to check.
         **/
        static size_t numBytes(PointerType pt) {
            switch (pt) {
                case WeaklyTypedPointer::UCHAR:
                case WeaklyTypedPointer::CHAR:
                    return sizeof(char);
                case WeaklyTypedPointer::USHORT:
                case WeaklyTypedPointer::SHORT:
                    return sizeof(short);
                case WeaklyTypedPointer::UINT:
                case WeaklyTypedPointer::INT:
                    return sizeof(int);
                case WeaklyTypedPointer::FLOAT:
                    return sizeof(float);
                case WeaklyTypedPointer::ULONG:
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

        PointerType _pointerType;   ///< Base data type of the pointer
        void* _pointer;             ///< Pointer to the data
    };

}

#endif // WEAKLYTYPEDPOINTER_H__
