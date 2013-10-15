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

#ifndef GENERICABSTRACTIMAGEREPRESENTATION_H__
#define GENERICABSTRACTIMAGEREPRESENTATION_H__

#include "core/datastructures/abstractimagerepresentation.h"
#include "core/datastructures/datacontainer.h"
#include "core/datastructures/imagedata.h"

namespace campvis {
    /**
     * Generic wrapper around an AbstractImageRepresentation that provides a ScopedRepresentation proxy.
     * The concept is very similar to the DataContainer::ScopedData proxy.
     * From the outside ScopedRepresentation behaves exactly like a const T*, but internally it preserves the
     * reference counting of a DataHandle. Use this class when you want temporary access to a specific
     * representation of an ImageData item in a DataContainer but don't want to do the dynamic_cast and 
     * the getRepresentation<T> yourself.
     *
     * \tparam  T   Base class of the ImageRepresentation to get.
     */
    template<typename T>
    class GenericAbstractImageRepresentation : public AbstractImageRepresentation {
    public:
        struct ScopedRepresentation {
            /**
             * Creates a new DataHandle to the data item with the key \a name in \a dc, that behaves like a T*.
             * \param   dc      DataContainer to grab data from
             * \param   name    Key of the DataHandle to search for
             */
            ScopedRepresentation(const DataContainer& dc, const std::string& name)
                : dh(dc.getData(name))
                , data(0)
                , representation(0) 
            {
                if (dh.getData() != 0) {
                    data = dynamic_cast<const ImageData*>(dh.getData());
                    if (data != 0) {
                        representation = data->getRepresentation<T>();
                    }
                }
            };

            /**
             * Creates a new ScopedRepresentation for the given DataHandle, that behaves like a T*.
             * \param   dh  Source DataHandle
             */
            ScopedRepresentation(DataHandle dataHandle)
                : dh(dataHandle)
                , data(0)
                , representation(0) 
            {
                if (dh.getData() != 0) {
                    data = dynamic_cast<const ImageData*>(dh.getData());
                    if (data != 0) {
                        representation = data->getRepresentation<T>();
                    }
                }
            };

            /**
             * Implicit conversion operator to const T*.
             * \return  The image representation of the image in the DataHandle, may be 0 when no DataHandle was found,
             *           the data is of the wrong type, or no suitable representation was found.
             */
            operator const T*() {
                return representation;
            }

            /**
             * Implicit arrow operator to const T*.
             * \return  The image representation of the image in the DataHandle, may be 0 when no DataHandle was found,
             *           the data is of the wrong type, or no suitable representation was found.
             */
            const T* operator->() const {
                return representation;
            }

            /**
             * Gets the DataHandle.
             * \return dh
             */
            DataHandle getDataHandle() const {
                return dh;
            }

            /**
             * Returns the parent ImageData of this image representation.
             * \return data
             */
            const ImageData* getImageData() const {
                return data;
            }

        private:
            /// Not copy-constructable
            ScopedRepresentation(const ScopedRepresentation& rhs);
            /// Not assignable
            ScopedRepresentation& operator=(const ScopedRepresentation& rhs);

            DataHandle dh;                  ///< DataHandle
            const ImageData* data;          ///< strongly-typed pointer to data, may be 0
            const T* representation;        ///< strongly-typed pointer to the image representation, may be 0
        };

        /**
         * Creates a new abstract representation for the image \a parent.
         * \param   parent  Image this representation represents, must not be 0.
         */
        GenericAbstractImageRepresentation(ImageData* parent)
            : AbstractImageRepresentation(parent)
        {
        }

        /**
         * Virtual Destructor.
         */
        virtual ~GenericAbstractImageRepresentation() {};

    };

}

#endif // GENERICABSTRACTIMAGEREPRESENTATION_H__
