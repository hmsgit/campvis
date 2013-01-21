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

#ifndef GENERICIMAGEREPRESENTATIONLOCAL_H__
#define GENERICIMAGEREPRESENTATIONLOCAL_H__

#include "core/datastructures/imagerepresentationlocal.h"
#include "core/tools/typetraits.h"

#include <cstring>  // needed for memcpy

namespace campvis {

    /**
     * Templated version of ImageRepresentationLocal, storing image data in the local memory.
     *
     * \sa      TypeTraits
     * \tparam  BASETYPE    Base type of the image data (type of a single channel of an image element)
     * \tparam  NUMCHANNELS Number of channels of the image data.
     */
    template<typename BASETYPE, size_t NUMCHANNELS>
    class GenericImageRepresentationLocal : public ImageRepresentationLocal {
    public:
        /// Type of one single image element
        typedef typename TypeTraits<BASETYPE, NUMCHANNELS>::ElementType ElementType;
        /// Type of this template instantiation
        typedef GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS> ThisType;

        /**
         * Template specialization of the ScopedRepresentation defined in GenericAbstractImageRepresentation<T>
         * since some compiler get confused by the nested templates.
         * \note    This is a simple copy and paste from GenericAbstractImageRepresentation<T> with
         *          explicitly named template parameter.
         */
        struct ScopedRepresentation {
            /**
             * Creates a new DataHandle to the data item with the key \a name in \a dc, that behaves like a const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>*.
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
                        representation = data->getRepresentation< GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS> >();
                    }
                }
            };

            /**
             * Implicit conversion operator to const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>*.
             * \return  The image representation of the image in the DataHandle, may be 0 when no DataHandle was found,
             *           the data is of the wrong type, or no suitable representation was found.
             */
            operator const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>*() {
                return representation;
            }

            /**
             * Implicit arrow operator to const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>*.
             * \return  The image representation of the image in the DataHandle, may be 0 when no DataHandle was found,
             *           the data is of the wrong type, or no suitable representation was found.
             */
            const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* operator->() const {
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
            const GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* representation;        ///< strongly-typed pointer to the image representation, may be 0
        };

        /**
         * Creates a new strongly typed ImageData object storing the image in the local memory.
         * 
         * \param   parent  Image this representation represents, must not be 0.
         * \param   data    Pointer to the image data, must not be 0, GenericImageRepresentationLocal takes ownership of this pointer!
         */
        GenericImageRepresentationLocal(const ImageData* parent, ElementType* data);

        /**
         * Destructor
         */
        virtual ~GenericImageRepresentationLocal();


        /**
         * Performs a conversion of \a source to an ImageRepresentationLocal if feasible.
         * Returns 0 if conversion was not successful or source representation type is not compatible.
         * \note    The caller has to take ownership of the returned pointer if not 0.
         * \param   source  Source image representation for conversion.
         * \return  A pointer to a local representation of \a source or 0 on failure. The caller has to take ownership.
         */
        static GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* tryConvertFrom(const AbstractImageRepresentation* source);

        /// \see AbstractImageRepresentation::clone()
        virtual ThisType* clone() const;

        /// \see AbstractImageRepresentation::getLocalMemoryFootprint()
        virtual size_t getLocalMemoryFootprint() const;

        /// \see AbstractImageRepresentation::getVideoMemoryFootprint()
        virtual size_t getVideoMemoryFootprint() const;

        /// \see AbstractImageRepresentation::getSubImage
        virtual ThisType* getSubImage(const ImageData* parent, const tgt::svec3& llf, const tgt::svec3& urb) const;

        /**
         * Returns a WeaklyTypedPointer to the image data.
         * \note    The pointer is still owned by this ImageRepresentationLocal. If you want a copy, use clone().
         * \return  A WeaklyTypedPointer to the image data.
         */
        virtual const WeaklyTypedPointer getWeaklyTypedPointer() const;

        /// \see ImageRepresentationLocal::getElementNormalized
        virtual float getElementNormalized(size_t index, size_t channel) const;

        /// \see ImageRepresentationLocal::getElementNormalized
        virtual float getElementNormalized(const tgt::svec3& position, size_t channel) const;

        /// \see ImageRepresentationLocal::getElementNormalizedLinear
        virtual float getElementNormalizedLinear(const tgt::vec3& position, size_t channel) const;

        /// \see ImageRepresentationLocal::setElementNormalized
        virtual void setElementNormalized(size_t index, size_t channel, float value);

        /// \see ImageRepresentationLocal::setElementNormalized
        virtual void setElementNormalized(const tgt::svec3& position, size_t channel, float value);


        /**
         * Returns the image element at the given index \a index.
         * \param   index   Array index of the image element to return.
         * \return  Image element at index \a index.
         */
        ElementType& getElement(size_t index);

        /**
         * Returns the image element at the given position in image space.
         * \param   position    Pixel/voxel coordinates of the image element to return.
         * \return  Image element at the coordinates \a position.
         */
        ElementType& getElement(const tgt::svec3& position);

        /**
         * Returns the image element at the given index \a index.
         * \param   index   Array index of the image element to return.
         * \return  Image element at index \a index.
         */
        const ElementType& getElement(size_t index) const;

        /**
         * Returns the image element at the given position in image space.
         * \param   position    Pixel/voxel coordinates of the image element to return.
         * \return  Image element at the coordinates \a position.
         */
        const ElementType& getElement(const tgt::svec3& position) const;

        /**
         * Sets the image element at the given index to the value \a value.
         * \param   index   Array index of the image element to change.
         * \param   value   New value of the specified image element.
         */
        void setElement(size_t index, const ElementType& value);

        /**
         * Sets the image element at the given position in image space to the value \a value.
         * \param   position    Pixel/voxel coordinates of the image element to change.
         * \param   value       New value of the specified image element.
         */
        void setElement(const tgt::svec3& position, const ElementType& value);

        /**
         * Returns a pointer to the image data.
         * \return  _data
         */
        ElementType* getImageData();

        /**
         * Returns a const pointer to the image data.
         * \return  _data
         */
        const ElementType* getImageData() const;


        /**
         * Returns the image element at the given coordinates \a position using bi-/trilinear filtering.
         * \param   position    Pixel/voxel coordinates of the image element to return.
         * \return  Bi-/Trilinear filtered image element at the specified coordinates.
         */
        ElementType getElementLinear(const tgt::vec3 position) const;

    protected:

        ElementType* _data;

    };

// = Template implementation ======================================================================

    template<typename BASETYPE, size_t NUMCHANNELS>
    campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::GenericImageRepresentationLocal(const ImageData* parent, ElementType* data)
        : ImageRepresentationLocal(parent, TypeTraits<BASETYPE, NUMCHANNELS>::weaklyTypedPointerBaseType)
        , _data(data)
    {
        tgtAssert(_parent->getNumChannels() == NUMCHANNELS, "Number of channels must match parent image's number of channels!");
        if (_data == 0) {
            size_t numElements = getNumElements();
            _data = new ElementType[numElements];
            memset(_data, 0, numElements * TypeTraits<BASETYPE, NUMCHANNELS>::elementSize);
        }
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::~GenericImageRepresentationLocal() {
        delete _data;
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::tryConvertFrom(const AbstractImageRepresentation* source) {
        return 0;
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::clone() const {
        size_t numElements = getNumElements();
        ElementType* newData = new ElementType[numElements];
        memcpy(newData, _data, numElements * sizeof(ElementType));

        return new ThisType(_parent, newData);
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    size_t campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getLocalMemoryFootprint() const {
        return sizeof(*this) + getNumElements() * sizeof(ElementType);
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    size_t campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getVideoMemoryFootprint() const {
        return 0;
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>* campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getSubImage(const ImageData* parent, const tgt::svec3& llf, const tgt::svec3& urb) const {
        tgtAssert(tgt::hand(tgt::lessThan(llf, urb)), "Coordinates in LLF must be component-wise smaller than the ones in URB!");

        const tgt::svec3& size = getSize();
        tgt::svec3 newSize = urb - llf;
        if (newSize == size) {
            // nothing has changed, just provide a copy:
            return clone();
        }

        tgt::bvec3 tmp(tgt::greaterThan(newSize, tgt::svec3(1)));
        size_t newDimensionality = 0;
        for (size_t i = 0; i < 3; ++i) {
            if (tmp[i] == true)
                ++newDimensionality;
        }

        size_t numBytesPerElement = sizeof(ElementType);
        size_t numElements = tgt::hmul(newSize);
        ElementType* newData = new ElementType[numElements];

        // slice image data into new array
        size_t index = 0;
        for (size_t z = llf.z; z < urb.z; ++z) {
            for (size_t y = llf.y; y < urb.y; ++y) {
                size_t offset = llf.x + (y * size.x) + (z * size.y * size.x);
                memcpy(newData + index, _data + offset, newSize.x * numBytesPerElement);
                index += newSize.x;
            }
        }        

        return new ThisType(parent, newData);
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    const WeaklyTypedPointer campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getWeaklyTypedPointer() const {
        return WeaklyTypedPointer(TypeTraits<BASETYPE, NUMCHANNELS>::weaklyTypedPointerBaseType, NUMCHANNELS, _data);
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    float campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getElementNormalized(size_t index, size_t channel) const {
        tgtAssert(channel >= 0 && channel < NUMCHANNELS, "Channel out of bounds!");
        return TypeNormalizer::normalizeToFloat(TypeTraits<BASETYPE, NUMCHANNELS>::getChannel(getElement(index), channel));
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    float campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getElementNormalized(const tgt::svec3& position, size_t channel) const {
        return getElementNormalized(_parent->positionToIndex(position), channel);
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    void campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::setElementNormalized(size_t index, size_t channel, float value) {
        tgtAssert(channel >= 0 && channel < NUMCHANNELS, "Channel out of bounds!");
        TypeTraits<BASETYPE, NUMCHANNELS>::setChannel(getElement(index), channel, TypeNormalizer::denormalizeFromFloat<BASETYPE>(value));
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    void campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::setElementNormalized(const tgt::svec3& position, size_t channel, float value) {
        setElementNormalized(_parent->positionToIndex(position), channel, value);
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    typename campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::ElementType& campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getElement(size_t position) {
        tgtAssert(position >= 0 && position < getNumElements(), "Position out of bounds!");
        return _data[position];
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    typename campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::ElementType& campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getElement(const tgt::svec3& position) {
        return getElement(_parent->positionToIndex(position));
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    const typename campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::ElementType& campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getElement(size_t position) const {
        tgtAssert(position >= 0 && position < getNumElements(), "Position out of bounds!");
        return _data[position];
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    const typename campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::ElementType& campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getElement(const tgt::svec3& position) const {
        return getElement(_parent->positionToIndex(position));
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    void campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::setElement(size_t position, const ElementType& value) {
        tgtAssert(position >= 0 && position < getNumElements(), "Position out of bounds!");
        _data[position] = value;

    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    void campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::setElement(const tgt::svec3& position, const ElementType& value) {
        _data[_parent->positionToIndex(position)] = value;
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    typename campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::ElementType* campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getImageData() {
        return _data;
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    const typename campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::ElementType* campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getImageData() const {
        return _data;
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    typename campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::ElementType campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getElementLinear(const tgt::vec3 position) const {
        // yet to be implemented
        // TODO: Check wether pixel/voxel coordinates lie on the edges or on the center of the pixels/voxels
        tgtAssert(false, "Yet to be implemented!");
        return ElementType(0);
    }


    template<typename BASETYPE, size_t NUMCHANNELS>
    float campvis::GenericImageRepresentationLocal<BASETYPE, NUMCHANNELS>::getElementNormalizedLinear(const tgt::vec3& position, size_t channel) const {
        tgt::vec3 posAbs = tgt::max(position - 0.5f, tgt::vec3::zero);
        tgt::vec3 p = posAbs - floor(posAbs); // get decimal part
        tgt::svec3 llb = tgt::svec3(posAbs);
        tgt::svec3 urf = tgt::svec3(ceil(posAbs));
        urf = min(urf, getSize() - tgt::svec3(1)); // clamp so the lookups do not exceed the dimensions
        llb = min(llb, getSize() - tgt::svec3(1)); // dito

        /*
            interpolate linearly
        */
        return  getElementNormalized(tgt::svec3(llb.x, llb.y, llb.z), channel) * (1.f-p.x)*(1.f-p.y)*(1.f-p.z) // llB
              + getElementNormalized(tgt::svec3(urf.x, llb.y, llb.z), channel) * (    p.x)*(1.f-p.y)*(1.f-p.z) // lrB
              + getElementNormalized(tgt::svec3(urf.x, urf.y, llb.z), channel) * (    p.x)*(    p.y)*(1.f-p.z) // urB
              + getElementNormalized(tgt::svec3(llb.x, urf.y, llb.z), channel) * (1.f-p.x)*(    p.y)*(1.f-p.z) // ulB
              + getElementNormalized(tgt::svec3(llb.x, llb.y, urf.z), channel) * (1.f-p.x)*(1.f-p.y)*(    p.z) // llF
              + getElementNormalized(tgt::svec3(urf.x, llb.y, urf.z), channel) * (    p.x)*(1.f-p.y)*(    p.z) // lrF
              + getElementNormalized(tgt::svec3(urf.x, urf.y, urf.z), channel) * (    p.x)*(    p.y)*(    p.z) // urF
              + getElementNormalized(tgt::svec3(llb.x, urf.y, urf.z), channel) * (1.f-p.x)*(    p.y)*(    p.z);// ulF
    }

}

#endif // GENERICIMAGEREPRESENTATIONLOCAL_H__