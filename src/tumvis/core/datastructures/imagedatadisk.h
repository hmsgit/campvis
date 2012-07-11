#ifndef IMAGEDATADISK_H__
#define IMAGEDATADISK_H__

#include "tgt/vector.h"
#include "core/datastructures/imagedata.h"

#include "core/tools/endianhelper.h"
#include "core/tools/typetraits.h"
#include "core/tools/weaklytypedpointer.h"

#include <fstream>
#include <string>

namespace TUMVis {

    /**
     * Subclass of ImageData offering access to image data stored in binary form on the local harddisk.
     * 
     * \todo    Number of channels
     */
    class ImageDataDisk : public ImageData {
    public:
        /**
         * Creates a new ImageData disk representation.
         *
         * \param url               Path to file with raw data
         * \param dimensionality    Dimensionality of data
         * \param size              Size of this image (number of elements per dimension)
         * \param type              Base type of data
         * \param offset            Offset of first data element in file (in bytes)
         * \param endianness        Endianess of data
         * \param stride            Number of _elemments_ _between_ adjacent elements for each dimension (\see ImageDataDisk::_stride).
         */
        ImageDataDisk(
            const std::string& url,
            size_t dimensionality,
            const tgt::svec3& size,
            WeaklyTypedPointer::BaseType type,
            size_t numChannels,
            size_t offset = 0,
            EndianHelper::Endianness endianness = EndianHelper::LITTLE_ENDIAN,
            const tgt::svec3& stride = tgt::svec3::zero
            );

        /**
         * Destructor
         */
        virtual ~ImageDataDisk();


        /**
         * \see AbstractData::clone()
         **/
        virtual ImageDataDisk* clone() const;

        /**
         * \see ImageData::getSubImage
         */
        virtual ImageDataDisk* getSubImage(const tgt::svec3& llf, const tgt::svec3& urb) const;


        /**
         * Downloads the whole image data to local memory.
         * \return  Pointer to the image data in the local memory.
         */
        WeaklyTypedPointer getImageData() const;

    private:
        /**
         * Calculates the canonical stride for the given image size.
         * \param size  Image size (number of elements per dimension).
         * \return      (0, size.x, size.x * size.y)
         */
        tgt::svec3 getCanonicStride(const tgt::svec3& size) const;

        std::string _url;                       ///< path to file with raw data
        size_t _offset;                         ///< offset of first data element in file (in bytes)
        WeaklyTypedPointer::BaseType _type;     ///< base type of data
        size_t _numChannels;                    ///< number of channels per element
        EndianHelper::Endianness _endianess;    ///< endianess of data
        
        /**
         * Number of _elemments_ _between_ adjacent elements for each dimension.
         * If the stride is 0 for a dimension, the elements are assumed to be tightly packed, hence for
         * the x-dimension 0, for the y-dimension _size.x and for the z-dimension (_size.x * _size.y).
         **/
        tgt::svec3 _stride;

        static const std::string loggerCat_;
    };

}

#endif // IMAGEDATADISK_H__
