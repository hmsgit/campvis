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

#include <tbb/tbb.h>
#include <tbb/spin_mutex.h>

#include "core/datastructures/genericimagerepresentationlocal.h"

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
        else DISPATCH_DISK_TO_GENERIC_LOCAL_CONVERSION(6)
        else {
            tgtAssert(false, "Should not reach this - wrong number of channel!");
            return 0;
        }
    }


}