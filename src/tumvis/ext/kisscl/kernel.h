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

        // TODO: samplers, buffers, etc.

    private:
        /**
         * Return the number of arguments to this kernel.
         * \return The number of arguments to this kernel. 
         */
        cl_uint getNumArgs() const;

        static const std::string loggerCat_;
    };


    template<class T>
    void kisscl::Kernel::setArgument(cl_uint index, const T& data) {
        return LCL_ERROR(clSetKernelArg(id_, index, KernelArgumentTypeTraits<T>::size(), KernelArgumentTypeTraits<T>::pointer(data)));
    }

}

#endif // KISSCL_KERNEL_H__
