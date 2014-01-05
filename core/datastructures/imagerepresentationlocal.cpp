// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2013, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "imagerepresentationlocal.h"

#include "tgt/glcontextmanager.h"
#include <tbb/tbb.h>
#include <tbb/spin_mutex.h>
#include "core/datastructures/imagerepresentationdisk.h"
#include "core/datastructures/imagerepresentationgl.h"
#include "core/datastructures/genericimagerepresentationlocal.h"
#include "core/tools/opengljobprocessor.h"
#include "core/tools/job.h"

#ifdef CAMPVIS_HAS_MODULE_ITK
#include "modules/itk/core/genericimagerepresentationitk.h"
#endif

#include <limits>

namespace campvis {
    
    const std::string ImageRepresentationLocal::loggerCat_ = "CAMPVis.core.datastructures.ImageRepresentationLocal";

    ImageRepresentationLocal::ImageRepresentationLocal(ImageData* parent, WeaklyTypedPointer::BaseType baseType)
        : GenericAbstractImageRepresentation<ImageRepresentationLocal>(parent)
        , _baseType(baseType)
    {
        _intensityRangeDirty = true;
    }

    ImageRepresentationLocal::~ImageRepresentationLocal() {

    }

    ImageRepresentationLocal* ImageRepresentationLocal::tryConvertFrom(const AbstractImageRepresentation* source) {
        if (source == 0)
            return 0;

        // test source image type via dynamic cast
        if (const ImageRepresentationDisk* tester = dynamic_cast<const ImageRepresentationDisk*>(source)) {
            return create(tester->getParent(), tester->getImageData());
        }
        else if (const ImageRepresentationGL* tester = dynamic_cast<const ImageRepresentationGL*>(source)) {
            ImageRepresentationLocal* toReturn = 0;

            if (GLJobProc.isCurrentThreadOpenGlThread()) {
                try {
                    WeaklyTypedPointer wtp = tester->getWeaklyTypedPointerCopy();
                    toReturn = create(source->getParent(), wtp);
                }
                catch (...) {
                    LERROR("An unknown error occured during conversion...");
                }

            }
            else {
                tgt::GLCanvas* context = GLJobProc.iKnowWhatImDoingGetArbitraryContext();
                GLJobProc.pause();
                try {
                    tgt::GLContextScopedLock lock(context);
                    WeaklyTypedPointer wtp = tester->getWeaklyTypedPointerCopy();
                    toReturn = create(source->getParent(), wtp);
                }
                catch (...) {
                    LERROR("An unknown error occured during conversion...");
                }
                GLJobProc.resume();
            }
            return toReturn;
        }

#ifdef CAMPVIS_HAS_MODULE_ITK
        // There is no way to determine basetype, number of channels and dimensionality of
        // an ITK image at runtime. So there are currently 7*4*2 = 56 different possibilities
        // what source could be. Thank god, there exists macro magic to create the 56
        // different templated conversion codes.
#define CONVERT_ITK_TO_GENERIC_LOCAL(basetype, numchannels, dimensionality) \
        if (const GenericImageRepresentationItk<basetype, 1, 3>* tester = dynamic_cast< const GenericImageRepresentationItk<basetype, 1, 3>* >(source)) { \
            typedef GenericImageRepresentationItk<basetype, 1, 3>::ItkImageType ImageType; \
            typedef ImageType::PixelType PixelType; \
            const PixelType* pixelData = tester->getItkImage()->GetBufferPointer(); \
            \
            ImageType::RegionType region; \
            region = tester->getItkImage()->GetBufferedRegion(); \
            \
            ImageType::SizeType s = region.GetSize(); \
            tgt::svec3 size(s[0], s[1], s[2]); \
            \
            PixelType* pixelDataCopy = new PixelType[tgt::hmul(size)]; \
            memcpy(pixelDataCopy, pixelData, tgt::hmul(size) * TypeTraits<basetype, 1>::elementSize); \
            return GenericImageRepresentationLocal<PixelType, 1>::create(const_cast<ImageData*>(source->getParent()), pixelDataCopy); \
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
#endif

        return 0;
    }

    const Interval<float>& ImageRepresentationLocal::getNormalizedIntensityRange() const {
        if (_intensityRangeDirty)
            computeNormalizedIntensityRange();

        return _normalizedIntensityRange;
    }

    void ImageRepresentationLocal::computeNormalizedIntensityRange() const {
        _normalizedIntensityRange = Interval<float>(); // reset interval to empty one
        tbb::spin_mutex _mutex; // mutex to protect for concurrent access

        tbb::parallel_for(tbb::blocked_range<size_t>(0, getNumElements()), [&] (const tbb::blocked_range<size_t>& range) {
            float localMin = std::numeric_limits<float>::max();
            float localMax = -std::numeric_limits<float>::max();

            for (size_t i = range.begin(); i != range.end(); ++i) {
                float value = this->getElementNormalized(i, 0);
                localMax = std::max(localMax, value);
                localMin = std::min(localMin, value);
            }

            {
                // TODO: there is probably a more elegant method...
                tbb::spin_mutex::scoped_lock(mutex);
                _normalizedIntensityRange.nibble(localMin);
                _normalizedIntensityRange.nibble(localMax);
            }
        });

        _intensityRangeDirty = false;
    }

    ImageRepresentationLocal* ImageRepresentationLocal::create(const ImageData* parent, WeaklyTypedPointer wtp) {
#define CONVERT_DISK_TO_GENERIC_LOCAL(baseType,numChannels) \
        return GenericImageRepresentationLocal<baseType, numChannels>::create( \
            const_cast<ImageData*>(parent), \
            reinterpret_cast< TypeTraits<baseType, numChannels>::ElementType*>(wtp._pointer));

#define DISPATCH_DISK_TO_GENERIC_LOCAL_CONVERSION(numChannels) \
        if (parent->getNumChannels() == (numChannels)) { \
            switch (wtp._baseType) { \
                case WeaklyTypedPointer::UINT8: \
                    CONVERT_DISK_TO_GENERIC_LOCAL(uint8_t, (numChannels)) \
                case WeaklyTypedPointer::INT8: \
                    CONVERT_DISK_TO_GENERIC_LOCAL(int8_t, (numChannels)) \
                case WeaklyTypedPointer::UINT16: \
                    CONVERT_DISK_TO_GENERIC_LOCAL(uint16_t, (numChannels)) \
                case WeaklyTypedPointer::INT16: \
                    CONVERT_DISK_TO_GENERIC_LOCAL(int16_t, (numChannels)) \
                case WeaklyTypedPointer::UINT32: \
                    CONVERT_DISK_TO_GENERIC_LOCAL(uint32_t, (numChannels)) \
                case WeaklyTypedPointer::INT32: \
                    CONVERT_DISK_TO_GENERIC_LOCAL(int32_t, (numChannels)) \
                case WeaklyTypedPointer::FLOAT: \
                    CONVERT_DISK_TO_GENERIC_LOCAL(float, (numChannels)) \
                default: \
                    tgtAssert(false, "Should not reach this - wrong base data type!"); \
                    return 0; \
            } \
        }

        DISPATCH_DISK_TO_GENERIC_LOCAL_CONVERSION(1)
        else DISPATCH_DISK_TO_GENERIC_LOCAL_CONVERSION(2)
        else DISPATCH_DISK_TO_GENERIC_LOCAL_CONVERSION(3)
        else DISPATCH_DISK_TO_GENERIC_LOCAL_CONVERSION(4)
        else {
            tgtAssert(false, "Should not reach this - wrong number of channel!");
            return 0;
        }
    }


}