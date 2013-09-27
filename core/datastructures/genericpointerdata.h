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

#ifndef GENERICPOINTERDATA_H__
#define GENERICPOINTERDATA_H__


#include "core/datastructures/abstractdata.h"
#include <string>

namespace campvis {

    /**
     * Class that generically wraps around a pointer of the template type and takes ownership of it.
     * \tparam  T   Type of the pointer this AbstractData wraps around.
     */
    template<typename T>
    class GenericPointerData : public AbstractData {
    public:
        /**
         * Creates a new GenericPointerData and initializes its pointer with \a data.
         * \param   data    The initial pointer for this data, may be 0, GenericPointerData takes ownerwhip.
         */
        explicit GenericPointerData(T* data)
            : AbstractData()
            , _data(data)
        {};

        /**
         * Destructor, deletes the pointer.
         */
        virtual ~GenericPointerData() {
            delete _data;
        };

        /**
         * Returns the pointer to the wrapped data.
         * \return  _data, may be 0, GenericPointerData has ownership
         */
        const T* getData() const {
            return _data;
        };

        /**
         * Returns the pointer to the wrapped data.
         * \return  _data, may be 0, GenericPointerData has ownership
         */
        T* getData() {
            return _data;
        };

        /**
         * Sets the data to \a data.
         * \param   data    The new pointer for this data, may be 0, GenericPointerData takes ownerwhip.
         */
        void setData(T* data) {
            _data = data;
        };


        /**
         * Prototype - clone method, some people call this virtual constructor...
         * \return  A SHALLOW copy of this object.
         */
        virtual AbstractData* clone() const {
            // FIXME: This is only a shallow copy - not what you expect from clone!
            return new GenericPointerData<T>(_data);
        };

        /**
         * Returns the local memory footprint of the data in bytes.
         * \return  Number of bytes occupied in local memory by the data.
         */
        virtual size_t getLocalMemoryFootprint() const {
            return sizeof(T) + sizeof(T*);
        };

        /**
         * Returns the video memory footprint of the data in bytes.
         * \return  Number of bytes occupied in video memory by the data.
         */
        virtual size_t getVideoMemoryFootprint() const {
            return 0;
        };

    protected:
        T* _data;           ///< Pointer to the data.
    };
}

#endif // GENERICPOINTERDATA_H__
