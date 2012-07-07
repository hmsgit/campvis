#ifndef IMAGEDATA_H__
#define IMAGEDATA_H__

#include "core/datastructures/abstractdata.h"
#include "core/datastructures/imagemappinginformation.h"
#include "tgt/vector.h"

namespace TUMVis {

    /**
     * Stores basic information about one (semantic) image of arbitrary dimension.
     * Different representations (e.g. local memory, OpenGL texture, OpenCL buffer) are
     * to be defined by inheritance.
     * 
     * \todo 
     */
    class ImageData : public AbstractData {
    public:
        ImageData(size_t dimensionality, const tgt::svec3& size);

        virtual ~ImageData();

        /**
         * Dimensionality of this image.
         * \return _dimensionality
         */
        size_t getDimensionality() const;

        /**
         * Size of this image (number of elements per dimension).
         * \return _size
         */
        const tgt::svec3& getSize() const;

        /**
         * Mapping information of this image
         * \return _mappingInformation
         */
        const ImageMappingInformation& getMappingInformation() const;

        /**
         * Mapping information of this image
         * \return _mappingInformation
         */
        ImageMappingInformation& getMappingInformation();

        /**
         * Returns the subimage of this image given by \a llf and \a urb.
         * TODO: Check whether it is necessary to adjust image mapping!
         *
         * \param   llf     Lower-Left-Front coordinates of subimage
         * \param   urb     Upper-Right-Back coordinates of subimage
         * \return  An image containing the subimage of this with the given coordinates.
         */
        virtual ImageData* getSubImage(const tgt::svec3& llf, const tgt::svec3& urb) const = 0;

    protected:
        size_t _dimensionality;                         ///< Dimensionality of this image
        tgt::svec3 _size;                               ///< Size of this image (number of elements per dimension)
        ImageMappingInformation _mappingInformation;    ///< Mapping information of this image

        static const std::string loggerCat_;
    };

}

#endif // IMAGEDATA_H__
