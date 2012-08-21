// ================================================================================================
// 
// This file is part of the KissCL, an OpenCL C++ wrapper following the KISS principle.
// 
// Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (software@cszb.net)
// 
// This library is free software; you can redistribute it and/or modify it under the terms of the 
// GNU Lesser General Public License version 3 as published by the Free Software Foundation.
// 
// This library is distributed in the hope that it will be useful, but WITHOUT ANY WARRANTY; 
// without even the implied warranty of MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See 
// the GNU Lesser General Public License for more details.
// 
// You should have received a copy of the GNU Lesser General Public License in the file 
// "LICENSE.txt" along with this library. If not, see <http://www.gnu.org/licenses/>.
// 
// ================================================================================================

#ifndef KISSCL_KERNEL_H__
#define KISSCL_KERNEL_H__

#include "tgt/vector.h"
#include "kisscl/kisscl.h"
#include "kisscl/memory.h"


namespace kisscl {

    namespace {
        /**
         * Traits for deducing the size and pointer for passing arguments to an OpenCL kernel.
         */
        template<class T>
        struct KernelArgumentTypeTraits {
        };

        /**
         * Traits for deducing the size and pointer for passing two-dimensional arguments to an OpenCL kernel.
         */
        template<class U>
        struct KernelArgumentTypeTraits< tgt::Vector2<U> > {
            /// Returns the size of the argument
            static size_t size(const tgt::Vector2<U>&) { return 2 * sizeof(U); };
            /// Returns a pointer to the argument
            static const U* pointer(const tgt::Vector2<U>& value) { return value.elem; };
        };
        
        /**
         * Traits for deducing the size and pointer for passing two-dimensional arguments to an OpenCL kernel.
         */
        template<class U>
        struct KernelArgumentTypeTraits< tgt::Vector3<U> > {
            /// Returns the size of the argument
            static size_t size(const tgt::Vector3<U>&) { return 3 * sizeof(U); };
            /// Returns a pointer to the argument
            static const U* pointer(const tgt::Vector3<U>& value) { return value.elem; };
        };
        
        /**
         * Traits for deducing the size and pointer for passing two-dimensional arguments to an OpenCL kernel.
         */
        template<class U>
        struct KernelArgumentTypeTraits< tgt::Vector4<U> > {
            /// Returns the size of the argument
            static size_t size(const tgt::Vector4<U>&) { return 4 * sizeof(U); };
            /// Returns a pointer to the argument
            static const U* pointer(const tgt::Vector4<U>& value) { return value.elem; };
        };
        
        // specialize integral types with template magic.
#define SPECIALIZE_KATT_INTEGRAL(type) \
    /** \
     * Traits for deducing the size and pointer for passing arguments to an OpenCL kernel. \
     */ \
    template<> \
    struct KernelArgumentTypeTraits<type> { \
        static size_t size() { return sizeof(type); }; \
        static const type* pointer(const type& value) { return &value; }; \
        }; \

        SPECIALIZE_KATT_INTEGRAL(cl_char);
        SPECIALIZE_KATT_INTEGRAL(cl_uchar);
        SPECIALIZE_KATT_INTEGRAL(cl_short);
        SPECIALIZE_KATT_INTEGRAL(cl_ushort);
        SPECIALIZE_KATT_INTEGRAL(cl_int);
        SPECIALIZE_KATT_INTEGRAL(cl_uint);
        SPECIALIZE_KATT_INTEGRAL(cl_long);
        SPECIALIZE_KATT_INTEGRAL(cl_ulong);
        SPECIALIZE_KATT_INTEGRAL(cl_float);
        SPECIALIZE_KATT_INTEGRAL(cl_double);

    }

    /**
     * Wrapper around an OpenCL kernel.
     */
    class Kernel : public CLWrapper<cl_kernel> {
    public:
        /**
         * Constructor
         * \param   id  Internal OpenCL handle of the kernel.
         */
        Kernel(cl_kernel id);

        /** 
         * Sets the kernel argument with index \a index to \a data.
         * \param   index   Argument index
         * \param   data    Data
         */
        template<class T>
        void setArgument(cl_uint index, const T& data);

        // Note: If the number of arguments is not enough, have a look at the handy Python script generating the templates.

        /**
         * Set the 1 arguments of the kernel.
         * \note    The number of arguments must match to the kernel.
         * \param   c0   Argument 1
         */
        template<class C0>
        void setArguments(C0 co);

        /**
         * Set the 2 arguments of the kernel.
         * \note    The number of arguments must match to the kernel.
         * \param   c0   Argument 1
         * \param   c1   Argument 2
         */
        template<class C0, class C1>
        void setArguments(C0 co, C1 c1);

        /**
         * Set the 3 arguments of the kernel.
         * \note    The number of arguments must match to the kernel.
         * \param   c0   Argument 1
         * \param   c1   Argument 2
         * \param   c2   Argument 3
         */
        template<class C0, class C1, class C2>
        void setArguments(C0 co, C1 c1, C2 c2);

        /**
         * Set the 4 arguments of the kernel.
         * \note    The number of arguments must match to the kernel.
         * \param   c0   Argument 1
         * \param   c1   Argument 2
         * \param   c2   Argument 3
         * \param   c3   Argument 4
         */
        template<class C0, class C1, class C2, class C3>
        void setArguments(C0 co, C1 c1, C2 c2, C3 c3);

        /**
         * Set the 5 arguments of the kernel.
         * \note    The number of arguments must match to the kernel.
         * \param   c0   Argument 1
         * \param   c1   Argument 2
         * \param   c2   Argument 3
         * \param   c3   Argument 4
         * \param   c4   Argument 5
         */
        template<class C0, class C1, class C2, class C3, class C4>
        void setArguments(C0 co, C1 c1, C2 c2, C3 c3, C4 c4);

        /**
         * Set the 6 arguments of the kernel.
         * \note    The number of arguments must match to the kernel.
         * \param   c0   Argument 1
         * \param   c1   Argument 2
         * \param   c2   Argument 3
         * \param   c3   Argument 4
         * \param   c4   Argument 5
         * \param   c5   Argument 6
         */
        template<class C0, class C1, class C2, class C3, class C4, class C5>
        void setArguments(C0 co, C1 c1, C2 c2, C3 c3, C4 c4, C5 c5);

        /**
         * Set the 7 arguments of the kernel.
         * \note    The number of arguments must match to the kernel.
         * \param   c0   Argument 1
         * \param   c1   Argument 2
         * \param   c2   Argument 3
         * \param   c3   Argument 4
         * \param   c4   Argument 5
         * \param   c5   Argument 6
         * \param   c6   Argument 7
         */
        template<class C0, class C1, class C2, class C3, class C4, class C5, class C6>
        void setArguments(C0 co, C1 c1, C2 c2, C3 c3, C4 c4, C5 c5, C6 c6);

        /**
         * Set the 8 arguments of the kernel.
         * \note    The number of arguments must match to the kernel.
         * \param   c0   Argument 1
         * \param   c1   Argument 2
         * \param   c2   Argument 3
         * \param   c3   Argument 4
         * \param   c4   Argument 5
         * \param   c5   Argument 6
         * \param   c6   Argument 7
         * \param   c7   Argument 8
         */
        template<class C0, class C1, class C2, class C3, class C4, class C5, class C6, class C7>
        void setArguments(C0 co, C1 c1, C2 c2, C3 c3, C4 c4, C5 c5, C6 c6, C7 c7);

        /**
         * Set the 9 arguments of the kernel.
         * \note    The number of arguments must match to the kernel.
         * \param   c0   Argument 1
         * \param   c1   Argument 2
         * \param   c2   Argument 3
         * \param   c3   Argument 4
         * \param   c4   Argument 5
         * \param   c5   Argument 6
         * \param   c6   Argument 7
         * \param   c7   Argument 8
         * \param   c8   Argument 9
         */
        template<class C0, class C1, class C2, class C3, class C4, class C5, class C6, class C7, class C8>
        void setArguments(C0 co, C1 c1, C2 c2, C3 c3, C4 c4, C5 c5, C6 c6, C7 c7, C8 c8);

        /**
         * Set the 10 arguments of the kernel.
         * \note    The number of arguments must match to the kernel.
         * \param   c0   Argument 1
         * \param   c1   Argument 2
         * \param   c2   Argument 3
         * \param   c3   Argument 4
         * \param   c4   Argument 5
         * \param   c5   Argument 6
         * \param   c6   Argument 7
         * \param   c7   Argument 8
         * \param   c8   Argument 9
         * \param   c9   Argument 10
         */
        template<class C0, class C1, class C2, class C3, class C4, class C5, class C6, class C7, class C8, class C9>
        void setArguments(C0 co, C1 c1, C2 c2, C3 c3, C4 c4, C5 c5, C6 c6, C7 c7, C8 c8, C9 c9);

    private:
        /**
         * Return the number of arguments to this kernel.
         * \return The number of arguments to this kernel. 
         */
        cl_uint getNumArgs() const;

        static const std::string loggerCat_;
    };

// ================================================================================================

    template<class T>
    void kisscl::Kernel::setArgument(cl_uint index, const T& data) {
        LCL_ERROR(clSetKernelArg(_id, index, KernelArgumentTypeTraits<T>::size(), KernelArgumentTypeTraits<T>::pointer(data)));
    }

    template<>
    void Kernel::setArgument(cl_uint index, const Buffer& data);

    template<>
    void Kernel::setArgument(cl_uint index, const Image& data);

    template<>
    void Kernel::setArgument(cl_uint index, const GLTexture& data);

// ================================================================================================

    template<class C0>
    void Kernel::setArguments(C0 c0) {
        tgtAssert(getInfo<cl_uint>(CL_KERNEL_NUM_ARGS) == 1, "Numer of arguments does not match the kernel's number of arguments.");

        setArgument<C0>(0, c0);
    }

    template<class C0, class C1>
    void Kernel::setArguments(C0 c0, C1 c1) {
        tgtAssert(getInfo<cl_uint>(CL_KERNEL_NUM_ARGS) == 2, "Numer of arguments does not match the kernel's number of arguments.");

        setArgument<C0>(0, c0);
        setArgument<C1>(1, c1);
    }

    template<class C0, class C1, class C2>
    void Kernel::setArguments(C0 c0, C1 c1, C2 c2) {
        tgtAssert(getInfo<cl_uint>(CL_KERNEL_NUM_ARGS) == 3, "Numer of arguments does not match the kernel's number of arguments.");

        setArgument<C0>(0, c0);
        setArgument<C1>(1, c1);
        setArgument<C2>(2, c2);
    }

    template<class C0, class C1, class C2, class C3>
    void Kernel::setArguments(C0 c0, C1 c1, C2 c2, C3 c3) {
        tgtAssert(getInfo<cl_uint>(CL_KERNEL_NUM_ARGS) == 4, "Numer of arguments does not match the kernel's number of arguments.");

        setArgument<C0>(0, c0);
        setArgument<C1>(1, c1);
        setArgument<C2>(2, c2);
        setArgument<C3>(3, c3);
    }

    template<class C0, class C1, class C2, class C3, class C4>
    void Kernel::setArguments(C0 c0, C1 c1, C2 c2, C3 c3, C4 c4) {
        tgtAssert(getInfo<cl_uint>(CL_KERNEL_NUM_ARGS) == 5, "Numer of arguments does not match the kernel's number of arguments.");

        setArgument<C0>(0, c0);
        setArgument<C1>(1, c1);
        setArgument<C2>(2, c2);
        setArgument<C3>(3, c3);
        setArgument<C4>(4, c4);
    }

    template<class C0, class C1, class C2, class C3, class C4, class C5>
    void Kernel::setArguments(C0 c0, C1 c1, C2 c2, C3 c3, C4 c4, C5 c5) {
        tgtAssert(getInfo<cl_uint>(CL_KERNEL_NUM_ARGS) == 6, "Numer of arguments does not match the kernel's number of arguments.");

        setArgument<C0>(0, c0);
        setArgument<C1>(1, c1);
        setArgument<C2>(2, c2);
        setArgument<C3>(3, c3);
        setArgument<C4>(4, c4);
        setArgument<C5>(5, c5);
    }

    template<class C0, class C1, class C2, class C3, class C4, class C5, class C6>
    void Kernel::setArguments(C0 c0, C1 c1, C2 c2, C3 c3, C4 c4, C5 c5, C6 c6) {
        tgtAssert(getInfo<cl_uint>(CL_KERNEL_NUM_ARGS) == 7, "Numer of arguments does not match the kernel's number of arguments.");

        setArgument<C0>(0, c0);
        setArgument<C1>(1, c1);
        setArgument<C2>(2, c2);
        setArgument<C3>(3, c3);
        setArgument<C4>(4, c4);
        setArgument<C5>(5, c5);
        setArgument<C6>(6, c6);
    }

    template<class C0, class C1, class C2, class C3, class C4, class C5, class C6, class C7>
    void Kernel::setArguments(C0 c0, C1 c1, C2 c2, C3 c3, C4 c4, C5 c5, C6 c6, C7 c7) {
        tgtAssert(getInfo<cl_uint>(CL_KERNEL_NUM_ARGS) == 8, "Numer of arguments does not match the kernel's number of arguments.");

        setArgument<C0>(0, c0);
        setArgument<C1>(1, c1);
        setArgument<C2>(2, c2);
        setArgument<C3>(3, c3);
        setArgument<C4>(4, c4);
        setArgument<C5>(5, c5);
        setArgument<C6>(6, c6);
        setArgument<C7>(7, c7);
    }

    template<class C0, class C1, class C2, class C3, class C4, class C5, class C6, class C7, class C8>
    void Kernel::setArguments(C0 c0, C1 c1, C2 c2, C3 c3, C4 c4, C5 c5, C6 c6, C7 c7, C8 c8) {
        tgtAssert(getInfo<cl_uint>(CL_KERNEL_NUM_ARGS) == 9, "Numer of arguments does not match the kernel's number of arguments.");

        setArgument<C0>(0, c0);
        setArgument<C1>(1, c1);
        setArgument<C2>(2, c2);
        setArgument<C3>(3, c3);
        setArgument<C4>(4, c4);
        setArgument<C5>(5, c5);
        setArgument<C6>(6, c6);
        setArgument<C7>(7, c7);
        setArgument<C8>(8, c8);
    }

    template<class C0, class C1, class C2, class C3, class C4, class C5, class C6, class C7, class C8, class C9>
    void Kernel::setArguments(C0 c0, C1 c1, C2 c2, C3 c3, C4 c4, C5 c5, C6 c6, C7 c7, C8 c8, C9 c9) {
        tgtAssert(getInfo<cl_uint>(CL_KERNEL_NUM_ARGS) == 10, "Numer of arguments does not match the kernel's number of arguments.");

        setArgument<C0>(0, c0);
        setArgument<C1>(1, c1);
        setArgument<C2>(2, c2);
        setArgument<C3>(3, c3);
        setArgument<C4>(4, c4);
        setArgument<C5>(5, c5);
        setArgument<C6>(6, c6);
        setArgument<C7>(7, c7);
        setArgument<C8>(8, c8);
        setArgument<C9>(9, c9);
    }



}

#endif // KISSCL_KERNEL_H__
