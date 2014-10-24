// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#include "imagerepresentationdisk.h"

#include "cgt/filesystem.h"


namespace campvis {
    const std::string ImageRepresentationDisk::loggerCat_ = "CAMPVis.core.datastructures.ImageRepresentationDisk";

    ImageRepresentationDisk* ImageRepresentationDisk::create(ImageData* parent, const std::string& url, WeaklyTypedPointer::BaseType type, size_t offset /*= 0*/, EndianHelper::Endianness endianness /*= EndianHelper::LITTLE_ENDIAN*/, const cgt::svec3& stride /*= cgt::svec3::zero */, bool multichannelSideBySide /*= false*/) {
        ImageRepresentationDisk* toReturn = new ImageRepresentationDisk(parent, url, type, offset, endianness, stride, multichannelSideBySide);
        toReturn->addToParent();
        return toReturn;
    }

    ImageRepresentationDisk::ImageRepresentationDisk(ImageData* parent, const std::string& url, WeaklyTypedPointer::BaseType type, size_t offset /*= 0*/, EndianHelper::Endianness endianness /*= EndianHelper::LITTLE_ENDIAN*/, const cgt::svec3& stride /*= cgt::svec2::zero */, bool multichannelSideBySide /*= false*/)
        : GenericAbstractImageRepresentation<ImageRepresentationDisk>(parent)
        , _url(url)
        , _offset(offset)
        , _type(type)
        , _endianess(endianness)
        , _stride(stride)
        , _multichannelSideBySide(multichannelSideBySide)
    {
    }

    ImageRepresentationDisk::~ImageRepresentationDisk() {
    }

    campvis::WeaklyTypedPointer ImageRepresentationDisk::getImageData() const {
        const cgt::svec3& size = getSize();
        size_t numElements = cgt::hmul(size);
        size_t numBytesPerElement = WeaklyTypedPointer::numBytes(_type, _parent->getNumChannels());
        size_t numBytes = numElements * numBytesPerElement;

        // open file and prepare for read
        std::ifstream file(_url.c_str(), std::ios::in | std::ios::binary | std::ios::ate);
        if (file.is_open()) {
            size_t fileSize = static_cast<size_t>(file.tellg());
            if (fileSize < numBytes) {
                LERROR("File is smaller than expected.");
                return WeaklyTypedPointer(_type, _parent->getNumChannels(), 0);
            }

            file.seekg(_offset, std::ios::beg);

            // Reserve memory - because we have no type information we simply read into a char array.
            char* data = new char[numBytes];

            // handle stride:
            cgt::svec3 canonicStride = getCanonicStride(size);
            if (_stride == cgt::svec3::zero || _stride == canonicStride) {
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
                        size_t sz = size.x*size.y;
                        for (size_t z = 0; z < size.z; ++z) {
                            size_t posZ = static_cast<size_t>(file.tellg());
                            file.read(data + index, sz * numBytesPerElement);
                            file.seekg(posZ + (_stride.z * numBytesPerElement), std::ios::beg);
                            index += sz;
                        }
                    }
                    else {
                        // we have a stride only in the last 2 dimensions so we can read the elements from the first dimension in a row
                        for (size_t z = 0; z < size.z; ++z) {
                            size_t posZ = static_cast<size_t>(file.tellg());
                            for (size_t y = 0; y < size.y; ++y) {
                                file.read(data + index, size.x * numBytesPerElement);
                                file.seekg((_stride.y - size.x) * numBytesPerElement, std::ios::cur);
                                index += size.x;
                            }
                            file.seekg(posZ + (_stride.z * numBytesPerElement), std::ios::beg);
                        }
                    }
                }
                else {
                    // read each element separately
                    for (size_t z = 0; z < size.z; ++z) {
                        size_t posZ = static_cast<size_t>(file.tellg());
                        for (size_t y = 0; y < size.y; ++y) {
                            size_t posY = static_cast<size_t>(file.tellg());
                            for (size_t x = 0; x < size.x; ++x) {    
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
                    case WeaklyTypedPointer::UINT8: // fallthrough
                    case WeaklyTypedPointer::INT8:
                        // nothing to do here.
                        break;

                    case WeaklyTypedPointer::UINT16: // fallthrough
                    case WeaklyTypedPointer::INT16: {
                        for (size_t i = 0; i < numElements; ++i)
                            EndianHelper::swapEndian<2>(data + (2*i));
                        break;
                        }

                    case WeaklyTypedPointer::UINT32: // fallthrough
                    case WeaklyTypedPointer::INT32: // fallthrough
                    case WeaklyTypedPointer::FLOAT: {
                        for (size_t i = 0; i < numElements; ++i)
                            EndianHelper::swapEndian<4>(data + (4*i));
                        break;
                        }

                    default:
                        cgtAssert(false, "Should not reach this!");
                        LERROR("Tried to swap endianess with unsupported number of bytes per element (" << numBytesPerElement << ")");
                        break;
                }
            }

            if (_multichannelSideBySide && _parent->getNumChannels() > 1) {
                // there is no simple in-place solution, so we copy everything around...
                char* reordered = new char[numBytes];
                size_t numBytesPerChannel = numBytesPerElement / _parent->getNumChannels();
                for (size_t i = 0; i < numElements; ++i) {
                    for (size_t j = 0; j < _parent->getNumChannels(); ++j) {
                        size_t fromOffset = (i + j*numElements) * numBytesPerChannel;
                        size_t toOffset = (j + i*_parent->getNumChannels()) * numBytesPerChannel;
                        memcpy(reordered + toOffset, data + fromOffset, numBytesPerChannel);
                    }
                }

                std::swap(data, reordered);
                delete [] reordered;
            }

            return WeaklyTypedPointer(_type, _parent->getNumChannels(), static_cast<void*>(data));
        }
        else {
            LERROR("Could not open file " << _url << " for reading.");
            return WeaklyTypedPointer(_type, _parent->getNumChannels(), 0);
        }

    }

    cgt::svec3 ImageRepresentationDisk::getCanonicStride(const cgt::svec3& size) const {
        return cgt::svec3(0, size.x, size.x * size.y);
    }

    ImageRepresentationDisk* ImageRepresentationDisk::clone(ImageData* newParent) const {
        return ImageRepresentationDisk::create(newParent, _url, _type, _offset, _endianess, _stride);
    }

    size_t ImageRepresentationDisk::getLocalMemoryFootprint() const {
        return sizeof(*this);
    }

    size_t ImageRepresentationDisk::getVideoMemoryFootprint() const {
        return 0;
    }

    WeaklyTypedPointer::BaseType ImageRepresentationDisk::getBaseType() const {
        return _type;
    }

    ImageRepresentationDisk* ImageRepresentationDisk::tryConvertFrom(const AbstractImageRepresentation* source) {
        // no conversion availible for now
        return 0;
    }

}