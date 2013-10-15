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

/// Anonymous namespace for templated endian swapping helper methods
namespace {
    // template
    template<typename T, size_t N>
    struct SwapHelper {
        static inline T swap(T value) {
            LERRORC("CAMPVis.Tools.EndianHelper", "Tried to call SwapHelper<T,N>::swap() with unsupported template arguments.");
            return value;
        }
    };

    template<typename T>
    struct SwapHelper<T, 2> {
        static inline T swap(T value) {
            return (value>>8) | (value<<8);
        }
    };

    template<typename T>
    struct SwapHelper<T, 4> {
        static inline T swap(T value) {
            return (value>>24) |
                ((value<<8) & 0x00FF0000) |
                ((value>>8) & 0x0000FF00) |
                (value<<24);
        }
    };

    template<>
    struct SwapHelper<float, 4> {
        static inline float swap(float value) {
            uint32_t tmp = SwapHelper<uint32_t, 4>::swap(*reinterpret_cast<uint32_t*>(&value));
            return (*reinterpret_cast<float*>(&tmp));
        }
    };

    template<typename T>
    struct SwapHelper<T, 8> {
        static inline T swap(T value) {
            return (value>>56) |
                ((value<<40) & 0x00FF000000000000) |
                ((value<<24) & 0x0000FF0000000000) |
                ((value<<8)  & 0x000000FF00000000) |
                ((value>>8)  & 0x00000000FF000000) |
                ((value>>24) & 0x0000000000FF0000) |
                ((value>>40) & 0x000000000000FF00) |
                (value<<56);
        }
    };

    template<>
    struct SwapHelper<double, 8> {
        static inline double swap(double value) {
            uint64_t tmp = SwapHelper<uint64_t, 4>::swap(*reinterpret_cast<uint64_t*>(&value));
            return (*reinterpret_cast<double*>(&tmp));
        }
    };
}

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
         * Performs endian-swapping of \a value in respect of its size.
         * \param   value   Value to perform endian-swapping on.
         * \return  Endian-swapped version of \a value.
         */
        template<typename T>
        static inline T swapEndian(T value) {
            return SwapHelper<T, sizeof(T)>::swap(value);
        }

    };
}

#endif
