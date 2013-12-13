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
                    if (data == 0 || representation == 0) {
                        dh = DataHandle(0);
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
                    if (data == 0 || representation == 0) {
                        dh = DataHandle(0);
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
