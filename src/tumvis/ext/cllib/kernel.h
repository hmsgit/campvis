#ifndef KERNEL_H__
#define KERNEL_H__

#include "tgt/vector.h"
#include "cllib/cllib.h"



namespace cllib {

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
    void cllib::Kernel::setArgument(cl_uint index, const T& data) {
        return LCL_ERROR(clSetKernelArg(id_, index, KernelArgumentTypeTraits<T>::size(), KernelArgumentTypeTraits<T>::pointer(data)));
    }

}

#endif // KERNEL_H__
