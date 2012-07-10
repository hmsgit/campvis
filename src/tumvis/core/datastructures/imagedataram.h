#ifndef IMAGEDATARAM_H__
#define IMAGEDATARAM_H__

#include "tgt/vector.h"
#include "core/datastructures/imagedata.h"

#include "core/tools/endianhelper.h"
#include "core/tools/typetraits.h"
#include "core/tools/weaklytypedpointer.h"

#include <cstring>
#include <fstream>
#include <string>

namespace TUMVis {

    /**
     * Templated class storing ImageData in the local memory. Each image element is of type \a T.
     * 
     * \note    Although you can use ImageDataRAM directly (which works perfectly well), it is 
     *          encouraged to use ImageDataRAMTraits for a clearer approach and better support 
     *          of the ImageData converters.
     * \sa      ImageDataRAMTraits
     * \todo    implement padding
     * \tparam  T   base class of elements
     */
    template<typename T>
    class ImageDataRAM : public ImageData {
    public:
        /**
         * Creates a new ImageData disk representation.
         *
         * \note    ImageDataRam takes ownership of \a data.
         * \param dimensionality    Dimensionality of data
         * \param size              Size of this image (number of elements per dimension)
         * \param data              Pointer to the image data, must not be 0, ImageDataRAM takes ownership of that pointer.
         */
        ImageDataRAM(
            size_t dimensionality, 
            const tgt::svec3& size,
            T* data = 0
            );

        /**
         * Destructor
         */
        virtual ~ImageDataRAM();


        /**
         * \see AbstractData::clone()
         **/
        virtual ImageDataRAM<T>* clone() const;

        /**
         * \see ImageData::getSubImage
         */
        virtual ImageDataRAM<T>* getSubImage(const tgt::svec3& llf, const tgt::svec3& urb) const;


        /**
         * Returns the WeaklyTypedPointer to the image data.
         * \return  Pointer to the image data.
         */
        T* getImageData();

        /**
         * Returns the WeaklyTypedPointer to the image data.
         * \return  Pointer to the image data.
         */
        const T* getImageData() const;

    protected:

        T* _data;               ///< pointer to image data

        static const std::string loggerCat_;
    };

// - Template implementation ----------------------------------------------------------------------

    template<typename T>
    TUMVis::ImageDataRAM<T>::ImageDataRAM(size_t dimensionality, const tgt::svec3& size, T* data) 
        : ImageData(dimensionality, size)
        , _data(data)
    {
    }

    template<typename T>
    TUMVis::ImageDataRAM<T>::~ImageDataRAM() {
        delete _data;
    }

    template<typename T>
    ImageDataRAM<T>* TUMVis::ImageDataRAM<T>::clone() const {
        size_t numElements = tgt::hmul(_size);
        T* newData = new T[numElements];
        memcpy(newData, _data, numElements * sizeof(T));

        return new ImageDataRAM<T>(_dimensionality, _size, newData);
    }

    template<typename T>
    ImageDataRAM<T>* TUMVis::ImageDataRAM<T>::getSubImage(const tgt::svec3& llf, const tgt::svec3& urb) const {
        tgtAssert(tgt::hand(tgt::lessThan(llf, urb)), "Coordinates in LLF must be component-wise smaller than the ones in URB!");

        tgt::svec3 newSize = urb - llf;
        if (newSize == _size) {
            // nothing has changed, just provide a copy:
            return clone();
        }

        size_t numBytesPerElement = sizeof(T);
        size_t numElements = tgt::hmul(_size);
        T* newData = new T[numElements];

        // slice image data into new array
        size_t index = 0;
        for (size_t z = llf.z; z < urb.z; ++z) {
            for (size_t y = llf.y; y < urb.y; ++y) {
                size_t offset = llf.x + (y * _size.x) + (z * _size.y * _size.x);
                memcpy(newData + index, _data + offset, newSize.x * numBytesPerElement);
                index += newSize.x;
            }
        }        

        return new ImageDataRAM<T>(_dimensionality, newSize, newData);
    }

    template<typename T>
    T* TUMVis::ImageDataRAM<T>::getImageData() {
        return _data;
    }

    template<typename T>
    const T* TUMVis::ImageDataRAM<T>::getImageData() const {
        return _data;
    }

    template<typename T>
    const std::string TUMVis::ImageDataRAM<T>::loggerCat_ = "TUMVis.core.datastructures.ImageDataRAM";

// - Traits and template specializations ----------------------------------------------------------

    /**
     * Collection of traits for ImageDataRAM<T> and its stored image data.
     * Although you can use ImageDataRAM directly (which works perfectly well), it is encouraged
     * to use these traits for a clearer approach and better support of the ImageData converters.
     * \sa      ImageDataRAM
     * \tparam  BASETYPE    Base data type
     * \tparam  NUMCHANNELS Number of channels of each element
     */
    template<typename BASETYPE, size_t NUMCHANNELS>
    struct ImageDataRAMTraits {
        
        /**
         * Returns the size of the element base type in bytes.
         */
        static const size_t basetypeSize = sizeof(BASETYPE);

        /**
         * Returns the number of channels per image element.
         */
        static const size_t numChannels = NUMCHANNELS;

        /**
         * Returns the number of bytes required for one image element.
         */
        static const size_t elementSize = sizeof(BASETYPE) * NUMCHANNELS;;
    };


    /**
     * Template specialization of ImageDataRAMTraits with one channel.
     * \sa      ImageDataRAMTraits, ImageDataRAM
     * \tparam  BASETYPE    Base data type
     */
    template<typename BASETYPE>
    struct ImageDataRAMTraits<BASETYPE, 1> {
        typedef ImageDataRAM<BASETYPE> ImageType;
    };

    /**
     * Template specialization of ImageDataRAMTraits with two channels.
     * \sa      ImageDataRAMTraits, ImageDataRAM
     * \tparam  BASETYPE    Base data type
     */
    template<typename BASETYPE>
    struct ImageDataRAMTraits<BASETYPE, 2> {
        typedef ImageDataRAM< tgt::Vector2<BASETYPE> > ImageType;
    };

    /**
     * Template specialization of ImageDataRAMTraits with three channels.
     * \sa      ImageDataRAMTraits, ImageDataRAM
     * \tparam  BASETYPE    Base data type
     */
    template<typename BASETYPE>
    struct ImageDataRAMTraits<BASETYPE, 3> {
        typedef ImageDataRAM< tgt::Vector3<BASETYPE> > ImageType;
    };

    /**
     * Template specialization of ImageDataRAMTraits with four channels.
     * \sa      ImageDataRAMTraits, ImageDataRAM
     * \tparam  BASETYPE    Base data type
     */
    template<typename BASETYPE>
    class ImageDataRAMTraits<BASETYPE, 4> {
        typedef ImageDataRAM< tgt::Vector4<BASETYPE> > ImageType;
    };

// - Convenience typedefs -------------------------------------------------------------------------

}

#endif // IMAGEDATARAM_H__
