// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universit�t M�nchen
//      Boltzmannstr. 3, 85748 Garching b. M�nchen, Germany
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

#ifndef GENERICIMAGEREPRESENTATIONITK_H__
#define GENERICIMAGEREPRESENTATIONITK_H__

#include "core/datastructures/abstractimagerepresentation.h"
#include "core/tools/typetraits.h"
#include "modules/itk/core/itktypetraits.h"

#include <itkImage.h>
#include <itkImportImageFilter.h>

#include <cstring>  // needed for memcpy

namespace campvis {

    /**
     * Templated image representation to hold an ITK image.
     *
     * \sa      TypeTraits
     * \tparam  BASETYPE    Base type of the image data (type of a single channel of an image element)
     * \tparam  NUMCHANNELS Number of channels of the image data.
     */
    template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
    class GenericImageRepresentationItk : public AbstractImageRepresentation {
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
        GenericImageRepresentationItk(ImageData* parent, typename ItkImageType::Pointer itkImage);

        /**
         * Destructor
         */
        virtual ~GenericImageRepresentationItk();


        /**
         * Performs a conversion of \a source to an GenericImageRepresentationItk<...> if feasible.
         * Returns 0 if conversion was not successful or source representation type is not compatible.
         * \note    The caller has to take ownership of the returned pointer if not 0.
         * \param   source  Source image representation for conversion.
         * \return  A pointer to a local representation of \a source or 0 on failure. The caller has to take ownership.
         */
        static ThisType* tryConvertFrom(const AbstractImageRepresentation* source);

        /// \see AbstractImageRepresentation::clone()
        virtual ThisType* clone(ImageData* newParent) const;

        /// \see AbstractImageRepresentation::getLocalMemoryFootprint()
        virtual size_t getLocalMemoryFootprint() const;

        /// \see AbstractImageRepresentation::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;

        /// \see AbstractImageRepresentation::getSubImage
        virtual ThisType* getSubImage(ImageData* parent, const tgt::svec3& llf, const tgt::svec3& urb) const;

        /**
         * Returns a WeaklyTypedPointer to the image data.
         * \note    The pointer is still owned by this ImageRepresentationLocal. If you want a copy, use clone().
         * \return  A WeaklyTypedPointer to the image data.
         */
        virtual const WeaklyTypedPointer getWeaklyTypedPointer() const;

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

        typename ItkImageType::Pointer _itkImage;

    };


// = Template implementation ======================================================================

    template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
    campvis::GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::GenericImageRepresentationItk(ImageData* parent, typename ItkImageType::Pointer itkImage)
        : AbstractImageRepresentation(parent)
        , _itkImage(itkImage)
    {
        tgtAssert(_parent->getNumChannels() == NUMCHANNELS, "Number of channels must match parent image's number of channels!");
        tgtAssert(_parent->getDimensionality() == DIMENSIONALITY, "The dimensionality must match parent image's dimensionality!");
    }

    template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
    campvis::GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::~GenericImageRepresentationItk() {
        
    }

    template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
    GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>* campvis::GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::tryConvertFrom(const AbstractImageRepresentation* source) {
        if (source == 0)
            return 0;

        if (source->getDimensionality() != DIMENSIONALITY) {
            LDEBUGC("campvis.modules.itk.core.GenericImageRepresentationItk<>::tryConvertFrom", "Dimensionality does not match");
            return 0;
        }

        if (const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* tester = dynamic_cast< const GenericImageRepresentationLocal<BASETYPE, 1>* >(source)) {
            itk::ImportImageFilter<BASETYPE, 3>::Pointer importer = itk::ImportImageFilter<BASETYPE, 3>::New();

            itk::Image<BASETYPE, 3>::SizeType size;
            size[0] = tester->getSize().x;
            size[1] = tester->getSize().y;
            size[2] = tester->getSize().z;

            itk::Image<BASETYPE, 3>::IndexType start;
            start.Fill(0);

            itk::Image<BASETYPE, 3>::RegionType region;
            region.SetSize(size);
            region.SetIndex(start);
            importer->SetRegion(region);

            importer->SetSpacing(tester->getParent()->getMappingInformation().getVoxelSize().elem);
            importer->SetOrigin(tester->getParent()->getMappingInformation().getOffset().elem);

            typedef typename itk::Image<BASETYPE, 3>::PixelType PixelType;
            const PixelType* pixelData = tester->getImageData();
            importer->SetImportPointer(const_cast<PixelType*>(pixelData), tester->getNumElements(), false);
            importer->Update();

            ItkImageType::Pointer itkImage = importer->GetOutput();
            if (itkImage.IsNotNull())
                return new ThisType(const_cast<ImageData*>(tester->getParent()), itkImage); // const_cast perfectly valid here
            else
                return 0;
        }

        return 0;
    }

    template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
    GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>* campvis::GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::clone(ImageData* newParent) const {
        ItkImageType::Pointer newItkImage = _itkImage->Clone();
        return new ThisType(newParent, newItkImage);
    }

    template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
    size_t campvis::GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::getLocalMemoryFootprint() const {
        // just an approximation, nobody knows it ITK internals...
        return sizeof(*this) + sizeof(ItkImageType) + getNumElements() * sizeof(ElementType);
    }

    template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
    size_t campvis::GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::getVideoMemoryFootprint() const {
        return 0;
    }

    template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
    GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>* campvis::GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::getSubImage(ImageData* parent, const tgt::svec3& llf, const tgt::svec3& urb) const {
        tgtAssert(tgt::hand(tgt::lessThan(llf, urb)), "Coordinates in LLF must be component-wise smaller than the ones in URB!");
        // TODO: implement - there certainly exists an ITK filter for this...
        return 0;
    }

    template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
    const WeaklyTypedPointer campvis::GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::getWeaklyTypedPointer() const {
        return WeaklyTypedPointer(TypeTraits<BASETYPE, NUMCHANNELS>::weaklyTypedPointerBaseType, NUMCHANNELS, _itkImage->GetBufferPointer());
    }

    template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
    typename GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::ItkImageType::Pointer campvis::GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::getItkImage() {
        return _itkImage;
    }

    template<typename BASETYPE, size_t NUMCHANNELS, size_t DIMENSIONALITY>
    typename GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::ItkImageType::ConstPointer campvis::GenericImageRepresentationItk<BASETYPE, NUMCHANNELS, DIMENSIONALITY>::getItkImage() const {
        return ItkImageType::ConstPointer(_itkImage);
    }
}

#endif // GENERICIMAGEREPRESENTATIONITK_H__