#ifndef IMAGEDATACONVERTER_H__
#define IMAGEDATACONVERTER_H__

#include "core/datastructures/imagedatadisk.h"
#include "core/datastructures/imagedatalocal.h"
#include "core/datastructures/genericimagedatalocal.h"
#include "core/datastructures/imagedatagl.h"

#include <string>

namespace TUMVis {

    /**
     * Static class offering conversion between differen ImageData types
     */
    struct ImageDataConverter {
    public:
        template<class TO>
        static TO* tryConvert(const ImageData* source);

        template<class FROM, class TO>
        static TO* convert(const FROM* source);

    protected:
        template<typename BASETYPE, size_t NUMCHANNELS>
        static GenericImageDataLocal<BASETYPE, NUMCHANNELS>* convertToGenericLocal(const ImageDataDisk* source);
        static const std::string loggerCat_;
    };

    template<class FROM, class TO>
    TO* TUMVis::ImageDataConverter::convert(const FROM* source) {
        tgtAssert(false, "Conversion not implemented.");
        return 0;
    }

    template<class TO>
    TO* TUMVis::ImageDataConverter::tryConvert(const ImageData* source) {
        if (source == 0)
            return 0;

        // test source image type via dynamic cast
        if (const ImageDataDisk* tester = dynamic_cast<const ImageDataDisk*>(source)) {
            return convert<ImageDataDisk, TO>(tester);
        }
        if (const ImageDataLocal* tester = dynamic_cast<const ImageDataLocal*>(source)) {
            return convert<ImageDataLocal, TO>(tester);
        }
        return 0;
    }

    template<>
    TUMVis::ImageDataLocal* ImageDataConverter::convert(const ImageDataDisk* source);

    template<>
    TUMVis::ImageDataGL* ImageDataConverter::convert(const ImageDataDisk* source);

    template<>
    TUMVis::ImageDataGL* ImageDataConverter::convert(const ImageDataLocal* source);


    template<typename BASETYPE, size_t NUMCHANNELS>
    TUMVis::GenericImageDataLocal<BASETYPE, NUMCHANNELS>* TUMVis::ImageDataConverter::convertToGenericLocal(const ImageDataDisk* source) {
        if (TypeTraits<BASETYPE, NUMCHANNELS>::basetypeSize != WeaklyTypedPointer::numBytes(source->getBaseType())) {
            LERROR("Basetype size does not match.");
            return 0;
        }
        if (NUMCHANNELS != source->getNumChannels()) {
            LERROR("Number of channels does not match.");
            return 0;
        }
        // TODO: check (un)signed int/float?

        WeaklyTypedPointer wtp = source->getImageData();
        return new GenericImageDataLocal<BASETYPE, NUMCHANNELS>(
            source->getDimensionality(), 
            source->getSize(), 
            reinterpret_cast<typename TypeTraits<BASETYPE, NUMCHANNELS>::ElementType*>(wtp._pointer));        
    }

}

#endif // IMAGEDATACONVERTER_H__
