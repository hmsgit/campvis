// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#ifndef mapping_h__
#define mapping_h__

namespace campvis {
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