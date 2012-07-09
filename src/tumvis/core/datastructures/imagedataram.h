#ifndef IMAGEDATARAM_H__
#define IMAGEDATARAM_H__

#include "tgt/vector.h"
#include "core/datastructures/imagedata.h"

#include "core/tools/endianhelper.h"
#include "core/tools/typetraits.h"
#include "core/tools/weaklytypedpointer.h"

#include <fstream>
#include <string>

namespace TUMVis {

    /**
     * Subclass of ImageData storing the image data in the local memory.
     * 
     * \todo    implement padding
     */
    class ImageDataRAM : public ImageData {
    public:
        /**
         * Creates a new ImageData disk representation.
         *
         * \param dimensionality    Dimensionality of data
         * \param size              Size of this image (number of elements per dimension)
         * \param data              WeaklyTypedPointer to the image data
         */
        ImageDataRAM(
            size_t dimensionality, 
            const tgt::svec3& size,
            WeaklyTypedPointer data
            );

        /**
         * Destructor
         */
        virtual ~ImageDataRAM();


        /**
         * \see AbstractData::clone()
         **/
        virtual ImageDataRAM* clone() const;

        /**
         * \see ImageData::getSubImage
         */
        virtual ImageDataRAM* getSubImage(const tgt::svec3& llf, const tgt::svec3& urb) const;


        /**
         * Returns the WeaklyTypedPointer to the image data.
         * \return  Pointer to the image data.
         */
        WeaklyTypedPointer& getImageData();

        /**
         * Returns the WeaklyTypedPointer to the image data.
         * \return  Pointer to the image data.
         */
        const WeaklyTypedPointer& getImageData() const;

    private:

        WeaklyTypedPointer _data;               ///< pointer to image data

        static const std::string loggerCat_;
    };

}

#endif // IMAGEDATARAM_H__
