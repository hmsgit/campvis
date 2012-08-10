// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

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
    struct LinearMapping {
    public:
        /**
         * Creates a new LinearMapping object
         * \param shift Shifting coefficient of mapping
         * \param scale Scaling coefficient of mapping
         * \return 
         */
        LinearMapping(T shift, T scale)
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
        static const LinearMapping<T> identity;
    };

    template<typename T>
    const LinearMapping<T> LinearMapping<T>::identity = LinearMapping<T>(T(0), T(1));
}

#endif // mapping_h__