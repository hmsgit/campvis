// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#include "imagedataconverter.h"

namespace campvis {
    const std::string ImageDataConverter::loggerCat_ = "CAMPVis.core.datastructures.ImageDataConverter";



    template<>
    campvis::ImageDataLocal* ImageDataConverter::convert(const ImageDataDisk* source) {
#define DISPATCH_CONVERSION(numChannels) \
        if (source->getNumChannels() == (numChannels)) { \
            switch (source->getBaseType()) { \
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
    campvis::ImageDataGL* ImageDataConverter::convert(const ImageDataDisk* source) {
        WeaklyTypedPointer wtp = source->getImageData();
        ImageDataGL* toReturn = new ImageDataGL(source->getDimensionality(), source->getSize(), wtp);
        toReturn->setMappingInformation(source->getMappingInformation());
        delete wtp._pointer;
        return toReturn;
    }

    template<>
    campvis::ImageDataGL* ImageDataConverter::convert(const ImageDataLocal* source) {
        ImageDataGL* toReturn = new ImageDataGL(source->getDimensionality(), source->getSize(), source->getWeaklyTypedPointer());
        toReturn->setMappingInformation(source->getMappingInformation());
        return toReturn;
    }

}