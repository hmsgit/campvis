// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
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

#include "imagerepresentationconversionitk.h"

#include "cgt/assert.h"
#include "cgt/opengljobprocessor.h"

namespace campvis {

    ImageRepresentationGL* GlFromItkConversion::tryConvertFrom(const AbstractImageRepresentation* source) {
        if (source == nullptr)
            return nullptr;

        if (const AbstractImageRepresentationItk* tester = dynamic_cast<const AbstractImageRepresentationItk*>(source)) {
            ImageRepresentationGL* toReturn = nullptr;
            GLJobProc.enqueueJobBlocking([&]() {
                toReturn = ImageRepresentationGL::create(const_cast<ImageData*>(tester->getParent()), tester->getWeaklyTypedPointer());
            });            
            return toReturn;
        }

        return nullptr;
    }


    ImageRepresentationLocal* LocalFromItkConversion::tryConvertFrom(const AbstractImageRepresentation* source) {
        if (source == nullptr)
            return nullptr;

        // There is no way to determine basetype, number of channels and dimensionality of
        // an ITK image at runtime. So there are currently 7*4*2 = 56 different possibilities
        // what source could be. Thank god, there exists macro magic to create the 56
        // different templated conversion codes.
#define CONVERT_ITK_TO_GENERIC_LOCAL(basetype, numchannels, dimensionality) \
        if (const GenericImageRepresentationItk<basetype, numchannels, dimensionality>* tester = dynamic_cast< const GenericImageRepresentationItk<basetype, numchannels, dimensionality>* >(source)) { \
            typedef GenericImageRepresentationItk<basetype, numchannels, dimensionality>::ItkImageType ItkImageType; \
            typedef ItkImageType::PixelType ItkElementType; \
            typedef GenericImageRepresentationItk<basetype, numchannels, dimensionality>::ElementType ElementType; \
            const ItkElementType* pixelData = tester->getItkImage()->GetBufferPointer(); \
            \
            ItkImageType::RegionType region; \
            region = tester->getItkImage()->GetBufferedRegion(); \
            \
            ItkImageType::SizeType s = region.GetSize(); \
            cgt::svec3 size(s[0], 1, 1); \
            if (dimensionality >= 2) \
                size.y = s[1]; \
            if (dimensionality == 3) \
                size.z = s[2]; \
            \
            ElementType* pixelDataCopy = new ElementType[cgt::hmul(size)]; \
            memcpy(pixelDataCopy, pixelData, cgt::hmul(size) * TypeTraits<basetype, numchannels>::elementSize); \
            return GenericImageRepresentationLocal<basetype, numchannels>::create(const_cast<ImageData*>(source->getParent()), pixelDataCopy); \
        }

#define DISPATCH_ITK_TO_GENERIC_LOCAL_CONVERSION_ND(numchannels, dimensionality) \
    CONVERT_ITK_TO_GENERIC_LOCAL(uint8_t, numchannels, dimensionality) \
    else CONVERT_ITK_TO_GENERIC_LOCAL(int8_t, numchannels, dimensionality) \
    else CONVERT_ITK_TO_GENERIC_LOCAL(uint16_t, numchannels, dimensionality) \
    else CONVERT_ITK_TO_GENERIC_LOCAL(int16_t, numchannels, dimensionality) \
    else CONVERT_ITK_TO_GENERIC_LOCAL(uint32_t, numchannels, dimensionality) \
    else CONVERT_ITK_TO_GENERIC_LOCAL(int32_t, numchannels, dimensionality) \
    else CONVERT_ITK_TO_GENERIC_LOCAL(float, numchannels, dimensionality)

#define DISPATCH_ITK_TO_GENERIC_LOCAL_CONVERSION_D(dimensionality) \
    DISPATCH_ITK_TO_GENERIC_LOCAL_CONVERSION_ND(1, dimensionality) \
    else DISPATCH_ITK_TO_GENERIC_LOCAL_CONVERSION_ND(2, dimensionality) \
    else DISPATCH_ITK_TO_GENERIC_LOCAL_CONVERSION_ND(3, dimensionality) \
    else DISPATCH_ITK_TO_GENERIC_LOCAL_CONVERSION_ND(4, dimensionality) 

        // okay we've defined our macros. Now we just need to call them so that they call 
        // each other and create 56 different conversion checks - hooray
        DISPATCH_ITK_TO_GENERIC_LOCAL_CONVERSION_D(2)
        else DISPATCH_ITK_TO_GENERIC_LOCAL_CONVERSION_D(3)

        return nullptr;
    }

}
