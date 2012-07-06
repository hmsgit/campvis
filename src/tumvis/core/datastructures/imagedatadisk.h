#ifndef imagedata_h__
#define imagedata_h__

#include "tgt/vector.h"
#include "datastructures/imagedata.h"
#include "tools/endianhelper.h"
#include "tools/typetraits.h"
#include <fstream>
#include <string>

namespace TUMVis {

    /**
     * Subclass of ImageData offering access to image data stored in binary form on the local harddisk.
     * 
     * \todo 
     */
    template<typename T>
    class ImageDataDisk : public ImageData {
    public:
        /**
         * Creates a new ImageData disk representation.
         *
         * \param url               Path to file with raw data
         * \param dimensionality    Dimensionality of data
         * \param size              Size of this image (number of elements per dimension)
         * \param offset            Offset of first data element in file (in bytes)
         * \param endianness        Endianess of data
         * \param stride            Number of _elemments_ _between_ adjacent elements for each dimension (\see ImageDataDisk::_stride).
         */
        ImageDataDisk(
            const std::string& url,
            size_t dimensionality, 
            const tgt::vec3& size,
            size_t offset = 0,
            EndianHelper::Endianness endianness = EndianHelper::LITTLE_ENDIAN,
            const tgt::svec3& stride = tgt::svec2::zero
            );

        /**
         * Destructor
         */
        virtual ~ImageDataDisk() {};

        /**
         * \see ImageData::getSubImage
         */
        virtual ImageDataDisk<T>* getSubImage(const tgt::vec3& llf, const tgt::vec3& urb);


        /**
         * Downloads the whole image data to local memory.
         * \return  Pointer to the image data in the local memory.
         */
        T* getImageData() const;

    private:
        /**
         * Calculates the canonical stride for the given image size.
         * \param size  Image size (number of elements per dimension).
         * \return      (0, size.x, size.x * size.y)
         */
        tgt::svec3 getCanonicStride(const tgt::svec3& size) const;

        std::string _url;                       ///< path to file with raw data
        size_t _offset;                         ///< offset of first data element in file (in bytes)
        EndianHelper::Endianness _endianess;    ///< endianess of data
        
        /**
         * Number of _elemments_ _between_ adjacent elements for each dimension.
         * If the stride is 0 for a dimension, the elements are assumed to be tightly packed, hence for
         * the x-dimension 0, for the y-dimension _size.x and for the z-dimension (_size.x * _size.y).
         **/
        tgt::svec3 _stride;

    };

    // - Template implementation ----------------------------------------------------------------------

    template<typename T>
    TUMVis::ImageDataDisk<T>::ImageDataDisk(const std::string& url, size_t dimensionality, const tgt::vec3& size, size_t offset /*= 0*/, EndianHelper::Endianness endianness /*= false*/, const tgt::svec3& stride /*= tgt::svec2::zero */) 
        : ImageData(dimensionality, size)
        , _url(url)
        , _offset(offset)
        , _endianess(endianness)
        , _stride(stride)
    {
    }

    template<typename T>
    ImageDataDisk<T>* TUMVis::ImageDataDisk<T>::getSubImage(const tgt::vec3& llf, const tgt::vec3& urb) {
        tgtAssert(tgt::hand(tgt::lessThan(llf, urb)), "Coordinates in LLF must be componentwise smaller than the ones in URB!");
        
        tgt::svec3 newSize = urb - llf;
        if (newSize == _size) {
            // nothing has changed, just provide a copy:
            return new ImageDataDisk<T>(_url, _dimensionality, newSize, _offset, _isBigEndian, _stride);
        }

        size_t newOffset = _offset + TypeTraits<T>::numBytes * llf.x;
        tgt::svec3 newStride = (_stride == tgt::svec3::zero) ? getCanonicStride(_size) : _stride;
        return new ImageDataDisk<T>(_url, _dimensionality, newSize, newOffset, _isBigEndian, newStride);
    }

    template<typename T>
    T* TUMVis::ImageDataDisk<T>::getImageData() const {
        size_t numElements = tgt::hmul(_size);
        size_t numBytes = numElements * TypeTraits<T>::numBytes;

        // open file and prepare for read
        std::ifstream file(_url, std::ios::in | std::ios::binary | std::ios::ate);
        if (file.is_open()) {
            size_t fileSize = file.tellg();
            if (fileSize < numBytes) {
                LERROR("File is smaller than expected.");
                return 0;
            }

            file.seekg(_offset, std::ios::beg);

            // handle stride:
            tgt::svec3 canonicStride = getCanonicStride(_size);
            if (_stride == tgt::svec3::zero || _stride == canonicStride) {
                // no stride is easy - we have just one chunk of data:
                T* data = new T[numElements];
                file.read(data, numBytes);
            }
            else {
                // TODO:    Check for correctness when being adequately awake! ;)
                size_t index = 0;
                size_t bytesPerElement = TypeTraits<T>::numBytes;

                // we have a stride at at least one direction
                if (_stride.z == 0 || canonicStride.z) {
                    if (_stride.y == 0 || canonicStride.y) {
                        // we have a stride only in the last dimension so we can read the elements from the first two dimension in a row
                        size_t sz = _size.x*_size.y;
                        for (size_t z = 0; z < _size.z; ++z) {
                            file.read(data + index, sz * bytesPerElement);
                            file.seekg((_stride.z - sz) * bytesPerElement, std::ios::cur);
                            index += sz;
                        }
                    }
                    else {
                        // we have a stride only in the last 2 dimensions so we can read the elements from the first dimension in a row
                        for (size_t z = 0; z < _size.z; ++z) {
                            for (size_t y = 0; y < _size.y; ++y) {
                                file.read(data + index, _size.x * bytesPerElement);
                                file.seekg((_stride.y - _size.x) * bytesPerElement, std::ios::cur);
                                index + _size.x;
                            }
                            file.seekg((_stride.z - (_size.x*_size.y)) * bytesPerElement, std::ios::cur);
                        }
                    }
                }
                else {
                    // read each element separately
                    for (size_t z = 0; z < _size.z; ++z) {
                        for (size_t y = 0; y < _size.y; ++y) {
                            for (size_t x = 0; x < _size.x; ++x) {    
                                file.read(data + index, bytesPerElement);
                                file.seekg((_stride.x - 1) * bytesPerElement, std::ios::cur);
                                ++index;
                            }
                            file.seekg((_stride.y - _size.x) * bytesPerElement, std::ios::cur);
                        }
                        file.seekg((_stride.z - (_size.x*_size.y)) * bytesPerElement, std::ios::cur);
                    }
                }
            }

            file.close();

            // handle endianess
            EndianHelper::Endianness localEndianess = EndianHelper::getLocalEndianness();
            if (_endianess != localEndianess) {
                for (size_t i = 0; i < numElements; ++i)
                    data[i] = EndianHelper::swapEndian(data[i]);
            }


            return data;
        }
        else {
            LERROR("Could not open file " << _url << " for reading.");
            return 0;
        }
    }

    template<typename T>
    tgt::svec3 TUMVis::ImageDataDisk<T>::getCanonicStride(const tgt::svec3& size) const {
        return tgt::svec3(0, size.x, size.x * size.y);
    }
}

#endif // imagedata_h__
