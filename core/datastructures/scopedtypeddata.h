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

#ifndef SCOPEDTYPEDDATA_H__
#define SCOPEDTYPEDDATA_H__

#include "core/datastructures/datacontainer.h"

namespace campvis {

    /**
     * Proxy class for scoped strongly-typed access to the data of a DataContainer.
     * From the outside ScopedTypedData<T> behaves exactly like a const T*, but internally it preserves the
     * reference counting of a DataHandle. Use this class when you want temporary access to a strongly-typed
     * data item in a DataContainer but don't want to to the dynamic_cast yourself.
     *
     * \tparam  T   Base class of the DataHandle data to test for
     */
    template<typename T>
    struct ScopedTypedData {
        /**
         * Creates a new DataHandle to the data item with the key \a name in \a dc, that behaves like a T*.
         * \param   dc      DataContainer to grab data from
         * \param   name    Key of the DataHandle to search for
         */
        ScopedTypedData(const DataContainer& dc, const std::string& name)
            : dh(dc.getData(name))
            , data(0)
        {
            if (dh.getData() != 0) {
                data = dynamic_cast<const T*>(dh.getData());
                if (data == 0)
                    dh = DataHandle(0);
            }
        };

        /**
         * Implicit conversion operator to const T*.
         * \return  The data in the DataHandle, may be 0 when no DataHandle was found, or the data is of the wrong type.
         */
        operator const T*() {
            return data;
        }

        /**
         * Implicit arrow operator to const T*.
         * \return  The data in the DataHandle, may be 0 when no DataHandle was found, or the data is of the wrong type.
         */
        const T* operator->() const {
            return data;
        }

        /**
         * Gets the DataHandle.
         * \return dh
         */
        const DataHandle& getDataHandle() const {
            return dh;
        }

    private:
        /// Not copy-constructable
        ScopedTypedData(const ScopedTypedData& rhs);
        /// Not assignable
        ScopedTypedData& operator=(const ScopedTypedData& rhs);

        DataHandle dh;      ///< DataHandle
        const T* data;      ///< strongly-typed pointer to data, may be 0
    };

}

#endif // SCOPEDTYPEDDATA_H__