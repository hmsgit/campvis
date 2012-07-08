#include "imagedatadisk.h"

namespace TUMVis {
    const std::string ImageDataDisk::loggerCat_ = "TUMVis.core.datastructures.ImageDataDisk";

    ImageDataDisk::ImageDataDisk(const std::string& url, size_t dimensionality, const tgt::svec3& size, WeaklyTypedPointer::PointerType type, size_t offset /*= 0*/, EndianHelper::Endianness endianness /*= EndianHelper::LITTLE_ENDIAN*/, const tgt::svec3& stride /*= tgt::svec2::zero */)
        : ImageData(dimensionality, size)
        , _url(url)
        , _offset(offset)
        , _endianess(endianness)
        , _stride(stride)
    {
    }

    ImageDataDisk::~ImageDataDisk() {

    }

    ImageData* ImageDataDisk::getSubImage(const tgt::svec3& llf, const tgt::svec3& urb) const {
        tgtAssert(tgt::hand(tgt::lessThan(llf, urb)), "Coordinates in LLF must be componentwise smaller than the ones in URB!");

        tgt::svec3 newSize = urb - llf;
        if (newSize == _size) {
            // nothing has changed, just provide a copy:
            return static_cast<ImageData*>(clone());
        }

        size_t newOffset = _offset + WeaklyTypedPointer::numBytes(_type) * llf.x;
        // the stride doesn't change!
        tgt::svec3 newStride = (_stride == tgt::svec3::zero) ? getCanonicStride(_size) : _stride;
        return new ImageDataDisk(_url, _dimensionality, newSize,_type, newOffset, _endianess, newStride);
    }

    TUMVis::WeaklyTypedPointer ImageDataDisk::getImageData() const {
        size_t numElements = tgt::hmul(_size);
        size_t numBytesPerElement = WeaklyTypedPointer::numBytes(_type);
        size_t numBytes = numElements * numBytesPerElement;

        // open file and prepare for read
        std::ifstream file(_url.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
        if (file.is_open()) {
            size_t fileSize = file.tellg();
            if (fileSize < numBytes) {
                LERROR("File is smaller than expected.");
                return WeaklyTypedPointer(_type, 0);
            }

            file.seekg(_offset, std::ios::beg);

            // Reserve memory - because we have no type information we simply read into a char array.
            char* data = new char[numBytes];

            // handle stride:
            tgt::svec3 canonicStride = getCanonicStride(_size);
            if (_stride == tgt::svec3::zero || _stride == canonicStride) {
                // no stride is easy - we have just one chunk of data:
                file.read(data, numBytes);
            }
            else {
                // TODO:    Check for correctness when being adequately awake! ;)
                size_t index = 0;

                // we have a stride at at least one direction
                if (_stride.z == 0 || canonicStride.z) {
                    if (_stride.y == 0 || canonicStride.y) {
                        // we have a stride only in the last dimension so we can read the elements from the first two dimension in a row
                        size_t sz = _size.x*_size.y;
                        for (size_t z = 0; z < _size.z; ++z) {
                            size_t posZ = static_cast<size_t>(file.tellg());
                            file.read(data + index, sz * numBytesPerElement);
                            file.seekg(posZ + (_stride.z * numBytesPerElement), std::ios::beg);
                            index += sz;
                        }
                    }
                    else {
                        // we have a stride only in the last 2 dimensions so we can read the elements from the first dimension in a row
                        for (size_t z = 0; z < _size.z; ++z) {
                            size_t posZ = static_cast<size_t>(file.tellg());
                            for (size_t y = 0; y < _size.y; ++y) {
                                file.read(data + index, _size.x * numBytesPerElement);
                                file.seekg((_stride.y - _size.x) * numBytesPerElement, std::ios::cur);
                                index += _size.x;
                            }
                            file.seekg(posZ + (_stride.z * numBytesPerElement), std::ios::beg);
                        }
                    }
                }
                else {
                    // read each element separately
                    for (size_t z = 0; z < _size.z; ++z) {
                        size_t posZ = static_cast<size_t>(file.tellg());
                        for (size_t y = 0; y < _size.y; ++y) {
                            size_t posY = static_cast<size_t>(file.tellg());
                            for (size_t x = 0; x < _size.x; ++x) {    
                                file.read(data + index, numBytesPerElement);
                                file.seekg((_stride.x - 1) * numBytesPerElement, std::ios::cur);
                                ++index;
                            }
                            file.seekg(posY + (_stride.y * numBytesPerElement), std::ios::beg);
                        }
                        file.seekg(posZ + (_stride.z * numBytesPerElement), std::ios::beg);
                    }
                }
            }

            file.close();

            // handle endianess
            EndianHelper::Endianness localEndianess = EndianHelper::getLocalEndianness();
            if (_endianess != localEndianess) {
                // This is not the most beautiful design, but unfortunately swapEndian needs to know the number of bytes at compiletime...
                switch (_type) {
                    case WeaklyTypedPointer::UCHAR:
                    case WeaklyTypedPointer::CHAR:
                        for (size_t i = 0; i < numElements; ++i)
                            data[i] = EndianHelper::swapEndian(data[i]);
                        break;

                    case WeaklyTypedPointer::USHORT:
                    case WeaklyTypedPointer::SHORT: {
                        short* tmp = reinterpret_cast<short*>(data);
                        for (size_t i = 0; i < numElements; ++i)
                            tmp[i] = EndianHelper::swapEndian(tmp[i]);
                        break;
                        }

                    case WeaklyTypedPointer::UINT:
                    case WeaklyTypedPointer::INT: {
                        int* tmp = reinterpret_cast<int*>(data);
                        for (size_t i = 0; i < numElements; ++i)
                            tmp[i] = EndianHelper::swapEndian(tmp[i]);
                        break;
                        }

                    case WeaklyTypedPointer::FLOAT: {
                        float* tmp = reinterpret_cast<float*>(data);
                        for (size_t i = 0; i < numElements; ++i)
                            tmp[i] = EndianHelper::swapEndian(tmp[i]);
                        break;
                        }

                    case WeaklyTypedPointer::ULONG:
                    case WeaklyTypedPointer::LONG: {
                        long* tmp = reinterpret_cast<long*>(data);
                        for (size_t i = 0; i < numElements; ++i)
                            tmp[i] = EndianHelper::swapEndian(tmp[i]);
                        break;
                        }

                    case WeaklyTypedPointer::DOUBLE: {
                        double* tmp = reinterpret_cast<double*>(data);
                        for (size_t i = 0; i < numElements; ++i)
                            tmp[i] = EndianHelper::swapEndian(tmp[i]);
                        break;
                        }

                    default:
                        tgtAssert(false, "Should not reach this!");
                        LERROR("Tried to swap endianess with unsupported number of bytes per element (" << numBytesPerElement << ")");
                        break;
                }
            }


            return WeaklyTypedPointer(_type, static_cast<void*>(data));
        }
        else {
            LERROR("Could not open file " << _url << " for reading.");
            return WeaklyTypedPointer(_type, 0);
        }

    }

    tgt::svec3 ImageDataDisk::getCanonicStride(const tgt::svec3& size) const {
        return tgt::svec3(0, size.x, size.x * size.y);
    }

    AbstractData* ImageDataDisk::clone() const {
        return new ImageDataDisk(_url, _dimensionality, _size, _type, _offset, _endianess, _stride);
    }
}