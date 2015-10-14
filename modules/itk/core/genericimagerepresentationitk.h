// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#ifndef GENERICIMAGEREPRESENTATIONITK_H__
#define GENERICIMAGEREPRESENTATIONITK_H__

#include "core/datastructures/abstractimagerepresentation.h"
#include "core/datastructures/imagedata.h"
#include "core/tools/weaklytypedpointer.h"
#include "modules/itk/core/itktypetraits.h"

#include <itkImage.h>

#include <cstring>  // needed for memcpy

namespace campvis {
    /**
     * Non templated base class for GenericImageRepresentationItk for easier RTTI check during runtime.
     */
    class AbstractImageRepresentationItk : public AbstractImageRepresentation {
    public:
        /// Virtual destructor
        virtual ~AbstractImageRepresentationItk() {};

        /**
         * Returns a WeaklyTypedPointer to the image data.
         * \note    The pointer is still owned by this ImageRepresentationLocal. If you want a copy, use clone().
         *          Please make sure not to mess with the pointer even if it's not const for technical reasons...
         * \return  A WeaklyTypedPointer to the image data.
         */
        virtual WeaklyTypedPointer getWeaklyTypedPointer() const = 0;

    protected:
        /**
         * Creates a new AbstractImageRepresentationItk. Just calls the base constructor.
         * \param   parent  Parent image data of this representation.
         */
        AbstractImageRepresentationItk(ImageData* parent)
            : AbstractImageRepresentation(parent)
        {};
    };

    /**
     * Templated image representation to hold an ITK image.
     *
     * \sa      TypeTraits
     * \tparam  BASETYPE    Base type of the image data (type of a single channel of an image element)
     * \tparam  NUMCHANNELS Number of channels of the image data.
     */
    template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
    class GenericImageRepresentationItk : public AbstractImageRepresentationItk {
    public:
        /// Type of one single image element
        typedef typename TypeTraits<BASETYPE, NUMCHANNELS>::ElementType ElementType;
        /// Type of one single image element
        typedef typename ItkTypeTraits<BASETYPE, NUMCHANNELS>::ItkElementType ItkElementType;
        /// Type of this template instantiation
        typedef GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY> ThisType;
        /// Typedef for the itk image this representation stores
        typedef itk::Image<ItkElementType, DIMENSIONALITY> ItkImageType;

        /**
         * Template specialization of the ScopedRepresentation defined in GenericAbstractImageRepresentation<T>
         * since some compiler get confused by the nested templates.
         * \note    This is a simple copy and paste from GenericAbstractImageRepresentation<T> with
         *          explicitly named template parameter.
         */
        struct ScopedRepresentation {
            /**
             * Creates a new DataHandle to the data item with the key \a name in \a dc, that behaves like a const GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>*.
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
                        representation = data->getRepresentation<ThisType>();
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
                        representation = data->getRepresentation<ThisType>();
                    }
                }
            };

            /**
             * Implicit conversion operator to const GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>*.
             * \return  The image representation of the image in the DataHandle, may be 0 when no DataHandle was found,
             *           the data is of the wrong type, or no suitable representation was found.
             */
            operator const ThisType*() {
                return representation;
            }

            /**
             * Implicit arrow operator to const GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>*.
             * \return  The image representation of the image in the DataHandle, may be 0 when no DataHandle was found,
             *           the data is of the wrong type, or no suitable representation was found.
             */
            const ThisType* operator->() const {
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

            DataHandle dh;                      ///< DataHandle
            const ImageData* data;              ///< strongly-typed pointer to data, may be 0
            const ThisType* representation;     ///< strongly-typed pointer to the image representation, may be 0
        };

        /**
         * Creates a new strongly typed ImageData object storing the image in the local memory.
         * 
         * \param   parent  Image this representation represents, must not be 0.
         * \param   data    Pointer to the image data, must not be 0, GenericImageRepresentationItk takes ownership of this pointer!
         */
        static GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>* create(ImageData* parent, typename ItkImageType::Pointer itkImage);

        /**
         * Destructor
         */
        virtual ~GenericImageRepresentationItk();


        /// \see AbstractImageRepresentation::clone()
        virtual ThisType* clone(ImageData* newParent) const;

        /// \see AbstractImageRepresentation::getLocalMemoryFootprint()
        virtual size_t getLocalMemoryFootprint() const;

        /// \see AbstractImageRepresentation::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;

        /**
         * Returns a WeaklyTypedPointer to the image data.
         * \note    The pointer is still owned by this ImageRepresentationLocal. If you want a copy, use clone().
         *          Please make sure not to mess with the pointer even if it's not const for technical reasons...
         * \return  A WeaklyTypedPointer to the image data.
         */
        virtual WeaklyTypedPointer getWeaklyTypedPointer() const;

        /**
         * Returns a pointer to the itk image.
         * \return  _itkImage
         */
        typename ItkImageType::Pointer getItkImage();

        /**
         * Returns a const pointer to the itk image.
         * \return  _itkImage
         */
        typename ItkImageType::ConstPointer getItkImage() const;

    protected:
        /**
         * Creates a new strongly typed ImageData object storing the image in the local memory.
         * 
         * \param   parent  Image this representation represents, must not be 0.
         * \param   data    Pointer to the image data, must not be 0, GenericImageRepresentationItk takes ownership of this pointer!
         */
        GenericImageRepresentationItk(ImageData* parent, typename ItkImageType::Pointer itkImage);

        typename ItkImageType::Pointer _itkImage;

    };

// = Template implementation ======================================================================

    template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
    GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>* campvis::GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::create(ImageData* parent, typename ItkImageType::Pointer itkImage) {
        ThisType* toReturn = new ThisType(parent, itkImage);
        toReturn->addToParent();
        return toReturn;
    }

    template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
    campvis::GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::GenericImageRepresentationItk(ImageData* parent, typename ItkImageType::Pointer itkImage)
        : AbstractImageRepresentationItk(parent)
        , _itkImage(itkImage)
    {
        cgtAssert(_parent->getNumChannels() == NUMCHANNELS, "Number of channels must match parent image's number of channels!");
        cgtAssert(_parent->getDimensionality() == DIMENSIONALITY, "The dimensionality must match parent image's dimensionality!");
    }

    template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
    campvis::GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::~GenericImageRepresentationItk() {
        
    }

    template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
    GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>* campvis::GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::clone(ImageData* newParent) const {
        typename ItkImageType::Pointer newItkImage = _itkImage->Clone();
        return ThisType::create(newParent, newItkImage);
    }

    template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
    size_t campvis::GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::getLocalMemoryFootprint() const {
        // just an approximation, nobody knows the ITK internals...
        return sizeof(*this) + sizeof(ItkImageType) + getNumElements() * sizeof(ElementType);
    }

    template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
    size_t campvis::GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::getVideoMemoryFootprint() const {
        return 0;
    }

    template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
    WeaklyTypedPointer campvis::GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::getWeaklyTypedPointer() const {
        return WeaklyTypedPointer(TypeTraits<BASETYPE, NUMCHANNELS>::weaklyTypedPointerBaseType, NUMCHANNELS, _itkImage->GetBufferPointer());
    }

    template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
    typename GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::ItkImageType::Pointer campvis::GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::getItkImage() {
        return _itkImage;
    }

    template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
    typename GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::ItkImageType::ConstPointer campvis::GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::getItkImage() const {
        return typename ItkImageType::ConstPointer(_itkImage);
    }
}

#endif // GENERICIMAGEREPRESENTATIONITK_H__
