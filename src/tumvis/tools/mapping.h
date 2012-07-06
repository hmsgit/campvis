#ifndef mapping_h__
#define mapping_h__

namespace TUMVis {
     /**
      * Struct for storing and handling linear mapping coefficients. 
      * Offers methods for applying the mapping and the inverse mapping.
      *
      * \tparam  T   Numeric type for mapping
      **/
    template<typename T>
    struct LinearMaping {
    public:
        /**
         * Creates a new LinearMaping object
         * \param shift Shifting coefficient of mapping
         * \param scale Scaling coefficient of mapping
         * \return 
         */
        LinearMaping(T shift, T scale)
            : _shift(shift)
            , _scale(scale)
        {};

        /**
         * Applies the linear mapping to \a value.
         * \param   value   value to apply linear mapping to
         * \return  (value + _shift) * _scale
         */
        T applyMapping(T value) const {
            return (value + _shift) * _scale;
        };

        /**
        * Applies the inverse linear mapping to \a value.
        * \param    value   value to apply linear mapping to
         * \return  (value - _shift) / _scale
         */
        T applyInverseMapping(T value) const {
            return (value - _shift) / _scale;
        }

        T _shift;   ///< Shifting coefficient of mapping
        T _scale;   ///< Scaling coefficient of mapping


        /**
         * Identity mapping (shift = 0, scale = 1).
         */
        static const LinearMaping<T> identity;
    };

    template<typename T>
    const LinearMaping<T> LinearMaping<T>::identity = LinearMaping<T>(T(0), T(1));
}

#endif // mapping_h__