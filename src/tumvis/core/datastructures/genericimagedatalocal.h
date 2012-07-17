#ifndef GENERICIMAGEDATALOCAL_H__
#define GENERICIMAGEDATALOCAL_H__

#include "tgt/vector.h"
#include "core/datastructures/imagedatalocal.h"
#include "core/tools/typetraits.h"

#include <cstring>  // needed for memcpy

namespace TUMVis {

    /**
     * Templated version of ImageDataLocal, storing image data in the local memory.
     *
     * \sa      TypeTraits
     * \tparam  BASETYPE    Base type of the image data (type of a single channel of an image element)
     * \tparam  NUMCHANNELS Number of channels of the image data.
     */
    template<typename BASETYPE, size_t NUMCHANNELS>
    class GenericImageDataLocal : public ImageDataLocal {
    public:
        /// Type of one single image element
        typedef typename TypeTraits<BASETYPE, NUMCHANNELS>::ElementType ElementType;
        /// Type of this template instantiation
        typedef GenericImageDataLocal<BASETYPE, NUMCHANNELS> ThisType;

        /**
         * Creates a new strongly typed ImageData object storing the image in the local memory.
         * 
         * \param   dimensionality  Dimensionality of data
         * \param   size            Size of this image (number of elements per dimension)
         * \param   data            Pointer to the image data, must not be 0, GenericImageDataLocal takes ownership of this pointer!
         */
        GenericImageDataLocal(size_t dimensionality, const tgt::svec3& size, ElementType* data);

        /**
         * Destructor
         */
        virtual ~GenericImageDataLocal();


        /// \see AbstractData::clone()
        virtual ThisType* clone() const;

        /// \see ImageData::getSubImage
        virtual ThisType* getSubImage(const tgt::svec3& llf, const tgt::svec3& urb) const;

        /// \see ImageDataLocal::getElementNormalized
        virtual float getElementNormalized(const tgt::svec3& position, size_t channel) const;

        /// \see ImageDataLocal::setElementNormalized
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
    TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::GenericImageDataLocal(size_t dimensionality, const tgt::svec3& size, ElementType* data)
        : ImageDataLocal(dimensionality, size, TypeTraits<BASETYPE, NUMCHANNELS>::weaklyTypedPointerBaseType, NUMCHANNELS)
        , _data(data)
    {
        tgtAssert(data != 0, "Pointer to image data must not be 0!");
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::~GenericImageDataLocal() {
        delete _data;
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    GenericImageDataLocal<BASETYPE, NUMCHANNELS>* TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::clone() const {
        size_t numElements = tgt::hmul(_size);
        ElementType* newData = new ElementType[numElements];
        memcpy(newData, _data, numElements * sizeof(ElementType));

        return new ThisType(_dimensionality, _size, newData);
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    GenericImageDataLocal<BASETYPE, NUMCHANNELS>* TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::getSubImage(const tgt::svec3& llf, const tgt::svec3& urb) const {
        tgtAssert(tgt::hand(tgt::lessThan(llf, urb)), "Coordinates in LLF must be component-wise smaller than the ones in URB!");

        tgt::svec3 newSize = urb - llf;
        if (newSize == _size) {
            // nothing has changed, just provide a copy:
            return clone();
        }

        size_t numBytesPerElement = sizeof(ElementType);
        size_t numElements = tgt::hmul(_size);
        ElementType* newData = new ElementType[numElements];

        // slice image data into new array
        size_t index = 0;
        for (size_t z = llf.z; z < urb.z; ++z) {
            for (size_t y = llf.y; y < urb.y; ++y) {
                size_t offset = llf.x + (y * _size.x) + (z * _size.y * _size.x);
                memcpy(newData + index, _data + offset, newSize.x * numBytesPerElement);
                index += newSize.x;
            }
        }        

        return new ThisType(_dimensionality, newSize, newData);
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    float TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::getElementNormalized(const tgt::svec3& position, size_t channel) const {
        tgtAssert(channel >= 0 && channel < NUMCHANNELS, "Channel out of bounds!");
        return TypeNormalizer::normalizeToFloat(TypeTraits<BASETYPE, NUMCHANNELS>::getChannel(getElement(position), channel));
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    void TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::setElementNormalized(const tgt::svec3& position, size_t channel, float value) {
        tgtAssert(channel >= 0 && channel < NUMCHANNELS, "Channel out of bounds!");
        TypeTraits<BASETYPE, NUMCHANNELS>::setChannel(getElement(position), channel, TypeNormalizer::denormalizeFromFloat<BASETYPE>(value));
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    typename TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::ElementType& TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::getElement(size_t position) {
        tgtAssert(position >= 0 && position < _numElements, "Position out of bounds!");
        return _data[position];
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    typename TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::ElementType& TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::getElement(const tgt::svec3& position) {
        return getElement(positionToIndex(position));
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    const typename TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::ElementType& TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::getElement(size_t position) const {
        tgtAssert(position >= 0 && position < _numElements, "Position out of bounds!");
        return _data[position];
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    const typename TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::ElementType& TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::getElement(const tgt::svec3& position) const {
        return getElement(positionToIndex(position));
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    void TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::setElement(size_t position, const ElementType& value) {
        tgtAssert(position >= 0 && position < _numElements, "Position out of bounds!");
        _data[position] = value;

    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    void TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::setElement(const tgt::svec3& position, const ElementType& value) {
        _data[positionToIndex(position)] = value;
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    typename TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::ElementType* TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::getImageData() {
        return _data;
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    const typename TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::ElementType* TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::getImageData() const {
        return _data;
    }

    template<typename BASETYPE, size_t NUMCHANNELS>
    typename TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::ElementType TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>::getElementLinear(const tgt::vec3 position) const {
        // yet to be implemented
        // TODO: Check wether pixel/voxel coordinates lie on the edges or on the center of the pixels/voxels
        tgtAssert(false, "Yet to be implemented!");
        return ElementType(0);
    }
}

#endif // GENERICIMAGEDATALOCAL_H__