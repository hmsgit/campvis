#ifndef typetraits_h__
#define typetraits_h__

#include <limits>

namespace TUMVis {

    /**
     * Struct offering various traits for handling and converting base types like char, int and float.
     * 
     **/
    template<typename T>
    struct TypeTraits {
        /// Size of the data type in bytes
        static const size_t numBytes;

        /// flag whether the data type is signed
        static const bool isSigned;

        static const T swapEndian(T value);
    };

// - Template implementation - let the evil template magic party begin ----------------------------

    template<typename T> const size_t TUMVis::TypeTraits<T>::numBytes = sizeof(T);

    template<typename T> const bool TUMVis::TypeTraits<T>::isSigned = std::numeric_limits<T>::is_signed;
}


#endif