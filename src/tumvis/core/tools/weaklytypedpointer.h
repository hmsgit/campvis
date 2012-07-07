#ifndef WEAKLYTYPEDPOINTER_H__
#define WEAKLYTYPEDPOINTER_H__

#include "tgt/assert.h"

namespace TUMVis {

    
    struct WeaklyTypedPointer {
        enum PointerType {
            UCHAR,
            CHAR,
            USHORT,
            SHORT,
            UINT,
            INT,
            FLOAT,
            ULONG,
            LONG,
            DOUBLE
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
         *
         *
         * \param	pt
         * \param	ptr
         **/
        WeaklyTypedPointer(PointerType pt, void* ptr)
            : _pointerType(pt)
            , _pointer(ptr)
        {
        };


        PointerType _pointerType;
        void* _pointer;
    };

}

#endif // WEAKLYTYPEDPOINTER_H__
