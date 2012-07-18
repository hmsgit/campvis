#include "imagedataconverter.h"

namespace TUMVis {
    const std::string ImageDataConverter::loggerCat_ = "TUMVis.core.datastructures.ImageDataConverter";



    template<>
    TUMVis::ImageDataLocal* ImageDataConverter::convert(const ImageDataDisk* source) {
        WeaklyTypedPointer wtp = source->getImageData();

#define DISPATCH_CONVERSION(numChannels) \
        if (source->getNumChannels() == (numChannels)) { \
            switch (wtp._baseType) { \
                case WeaklyTypedPointer::UINT8: \
                    return convertToGenericLocal<uint8_t, (numChannels)>(source); \
                case WeaklyTypedPointer::INT8: \
                    return convertToGenericLocal<int8_t, (numChannels)>(source); \
                case WeaklyTypedPointer::UINT16: \
                    return convertToGenericLocal<uint16_t, (numChannels)>(source); \
                case WeaklyTypedPointer::INT16: \
                    return convertToGenericLocal<int16_t, (numChannels)>(source); \
                case WeaklyTypedPointer::UINT32: \
                    return convertToGenericLocal<uint32_t, (numChannels)>(source); \
                case WeaklyTypedPointer::INT32: \
                    return convertToGenericLocal<int32_t, (numChannels)>(source); \
                case WeaklyTypedPointer::FLOAT: \
                    return convertToGenericLocal<float, (numChannels)>(source); \
                default: \
                    tgtAssert(false, "Should not reach this - wrong base data type!"); \
                    return 0; \
            } \
        }

        DISPATCH_CONVERSION(1)
        else DISPATCH_CONVERSION(2)
        else DISPATCH_CONVERSION(3)
        else DISPATCH_CONVERSION(4)
        else {
            tgtAssert(false, "Should not reach this - wrong number of channel!");
            return 0;
        }
    }

    template<>
    TUMVis::ImageDataGL* ImageDataConverter::convert(const ImageDataDisk* source) {
        WeaklyTypedPointer wtp = source->getImageData();
        ImageDataGL* toReturn = new ImageDataGL(source->getDimensionality(), source->getSize(), wtp);
        delete wtp._pointer;
        return toReturn;
    }

    template<>
    TUMVis::ImageDataGL* ImageDataConverter::convert(const ImageDataLocal* source) {
        return new ImageDataGL(source->getDimensionality(), source->getSize(), source->getWeaklyTypedPointer());
    }

}