// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
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

#ifndef endianhelper_h__
#define endianhelper_h__

#include "tgt/logmanager.h"
#include "tgt/types.h"

#include <utility>

namespace campvis {

    /**
     * Helper struct for handling endianess.
     **/
    struct EndianHelper {
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
