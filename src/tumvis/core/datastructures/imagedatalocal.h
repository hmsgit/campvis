#ifndef IMAGEDATALOCAL_H__
#define IMAGEDATALOCAL_H__

#include "tgt/vector.h"
#include "core/datastructures/imagedata.h"

#include "core/tools/endianhelper.h"
#include "core/tools/weaklytypedpointer.h"

#include <cstring>
#include <fstream>
#include <string>

namespace TUMVis {

    /**
     * Abstract base class for strongly typed (templated) ImageDataRAM
     * 
     * \todo    implement padding, add some kind of cool iterators
     */
    class ImageDataLocal : public ImageData {
    public:
        /**
         * Creates a new ImageData representation in local memory.
         *
         * \param   dimensionality  Dimensionality of data
         * \param   size            Size of this image (number of elements per dimension)
         */
        ImageDataLocal(size_t dimensionality, const tgt::svec3& size, WeaklyTypedPointer::BaseType baseType, size_t numChannels);

        /**
         * Destructor
         */
        virtual ~ImageDataLocal();


        /**
         * \see AbstractData::clone()
         **/
        virtual ImageDataLocal* clone() const = 0;

        /**
         * \see ImageData::getSubImage
         */
        virtual ImageDataLocal* getSubImage(const tgt::svec3& llf, const tgt::svec3& urb) const = 0;


        /**
         * Returns the WeaklyTypedPointer to the image data.
         * \return  Pointer to the image data.
         */
        virtual WeaklyTypedPointer& getImageData() = 0;

        /**
         * Returns the WeaklyTypedPointer to the image data.
         * \return  Pointer to the image data.
         */
        virtual const WeaklyTypedPointer& getImageData() const = 0;


        /**
         * Returns the normalized value of the element at the given position and channel.
         *  - for \em unsigned integer types, the value range is mapped linearly to [0.0;1.0]
         *  - for \em signed integer types, the value range is mapped linearly to [-1.0;1.0]
         *  - floating point types are not mapped
         * 
         * Simple algorithms on images might not always want to test for the actual base data type.
         * For them access to the normalized element values provided here might be enough.
         * 
         * \note    This method is virtual => slow!
         * \sa      ImageDataLocal::setElementNormalized
         * \param   position    Element position within the image
         * \param   channel     Image channel
         * \return  A normalized float representation of the element at the given position and channel.
         */
        virtual float getElementNormalized(const tgt::svec3& position, size_t channel) const = 0;

        /**
         * Sets the element at the given position and channel denormalized from the given value \a value.
         *  - for \em unsigned integer types, the value range is mapped linearly to [0.0;1.0]
         *  - for \em signed integer types, the value range is mapped linearly to [-1.0;1.0]
         *  - floating point types are not mapped
         * 
         * Simple algorithms on images might not always want to test for the actual base data type.
         * For them access to the normalized element values provided here might be enough.
         * 
         * \note    This method is virtual => slow!
         * \sa      ImageDataLocal::getElementNormalized
         * \param   position    Element position within the image
         * \param   channel     Image channel
         * \param   value       Normalized Value to set
         */
        virtual void setElementNormalized(const tgt::svec3& position, size_t channel, float value) = 0;

    protected:
        WeaklyTypedPointer::BaseType _baseType;
        size_t _numChannels;

        static const std::string loggerCat_;

    private:
        // We don't want this data to be copied - clone() must be enough
        // (read: We are too lazy to implement a correct copy constructor / assignment-operator)
         ImageDataLocal(const ImageDataLocal& rhs);
         ImageDataLocal& operator=(const ImageDataLocal& rhs);
    };


}

#endif // IMAGEDATALOCAL_H__
