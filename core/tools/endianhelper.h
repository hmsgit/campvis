// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
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

#ifndef endianhelper_h__
#define endianhelper_h__

#include "cgt/logmanager.h"
#include "cgt/types.h"

#include "core/coreapi.h"

#include <utility>

namespace campvis {

    /**
     * Helper struct for handling endianess.
     **/
    struct CAMPVIS_CORE_API EndianHelper {
        /// Type of Endianness
        enum Endianness {
            IS_LITTLE_ENDIAN,  ///< little endian
            IS_BIG_ENDIAN      ///< big endian
        };

        /**
         * Returns the Endianess of the local system.
         * \return Endianess of this very system.
         */
        static Endianness getLocalEndianness() {
            union {
                uint32_t i;
                char c[4];
            } bint = {0x01020304};
            return (bint.c[0] == 1) ? IS_BIG_ENDIAN : IS_LITTLE_ENDIAN;
        }

        /**
         * Performs in-place endian-swapping of value pointed to by \a value, supposing its size to be \a N bytes.
         * \param   value   Pointer to the value to be endian-swapped.
         * \tparam  N       Number of bytes of the value to be swapped.
         */
        template<size_t N>
        static inline void swapEndian(char* value) {
            for (size_t i = 0; i < N/2; ++i) {
                std::swap(value[i], value[N-i-1]);
            }
        }

    };
}

#endif
