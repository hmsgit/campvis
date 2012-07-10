#include "imagedataram.h"


namespace TUMVis {

/*
    const std::string ImageDataRAM::loggerCat_ = "TUMVis.core.datastructures.ImageDataRAM";

    ImageDataRAM::ImageDataRAM(size_t dimensionality, const tgt::svec3& size, WeaklyTypedPointer data)
        : ImageData(dimensionality, size)
        , _data(data)
    {

        Full<int, 2> f;
        f.foo(tgt::ivec2(1, 2));
    }

    ImageDataRAM::~ImageDataRAM() {
        delete _data._pointer;
    }

    ImageDataRAM* ImageDataRAM::clone() const {
        size_t numBytes = tgt::hmul(_size) * _data.numBytes();
        char* newData = new char[numBytes];
        memcpy(newData, _data._pointer, numBytes);

        return new ImageDataRAM(_dimensionality, _size, WeaklyTypedPointer(_data._pointerType, static_cast<void*>(newData)));
    }

    ImageDataRAM* ImageDataRAM::getSubImage(const tgt::svec3& llf, const tgt::svec3& urb) const {
        tgtAssert(tgt::hand(tgt::lessThan(llf, urb)), "Coordinates in LLF must be component-wise smaller than the ones in URB!");

        tgt::svec3 newSize = urb - llf;
        if (newSize == _size) {
            // nothing has changed, just provide a copy:
            return clone();
        }

        size_t numBytesPerElement = _data.numBytes();
        size_t numBytesTotal = tgt::hmul(newSize) * numBytesPerElement;
        char* newData = new char[numBytesTotal];

        // slice image data into new array
        size_t index = 0;
        for (size_t z = llf.z; z < urb.z; ++z) {
            for (size_t y = llf.y; y < urb.y; ++y) {
                size_t offset = llf.x + (y * _size.x) + (z * _size.y * _size.x);
                memcpy(newData + (index * numBytesPerElement), static_cast<char*>(_data._pointer) + (offset * numBytesPerElement), newSize.x * numBytesPerElement);
                index += newSize.x;
            }
        }        

        return new ImageDataRAM(_dimensionality, newSize, WeaklyTypedPointer(_data._pointerType, static_cast<void*>(newData)));
    }

    WeaklyTypedPointer& ImageDataRAM::getImageData() {
        return _data;
    }

    const WeaklyTypedPointer& ImageDataRAM::getImageData() const {
        return _data;
    }*/
}