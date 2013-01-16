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

#include "imagerepresentationlocal.h"

#include "tbb/include/tbb/tbb.h"
#include "tbb/include/tbb/spin_mutex.h"
#include <limits>

namespace campvis {
    
    class NormalizedIntensityRangeGenerator {
    public:
        NormalizedIntensityRangeGenerator(const ImageRepresentationLocal* intensityData, Interval<float>* interval)
            : _intensityData(intensityData)
            , _interval(interval)
        {
            *_interval = Interval<float>();
        }

        void operator() (const tbb::blocked_range<size_t>& range) const {
            float localMin = std::numeric_limits<float>::max();
            float localMax = -std::numeric_limits<float>::max();

            for (size_t i = range.begin(); i != range.end(); ++i) {
                float value = _intensityData->getElementNormalized(i, 0);
                localMax = std::max(localMax, value);
                localMin = std::min(localMin, value);
            }

            {
                // TODO: there is probably a more elegant method...
                tbb::spin_mutex::scoped_lock(_mutex);
                _interval->nibble(localMin);
                _interval->nibble(localMax);
            }
        }

    protected:
        const ImageRepresentationLocal* _intensityData;
        Interval<float>* _interval;
        tbb::spin_mutex _mutex;
    };

    // ================================================================================================

    class IntensityHistogramGenerator {
    public:
        IntensityHistogramGenerator(const ImageRepresentationLocal* intensityData, ImageRepresentationLocal::IntensityHistogramType* histogram)
            : _intensityData(intensityData)
            , _histogram(histogram)
        {}

        void operator() (const tbb::blocked_range<size_t>& range) const {
            for (size_t i = range.begin(); i != range.end(); ++i) {
                float value = _intensityData->getElementNormalized(i, 0);
                _histogram->addSample(&value);
            }
        }

    protected:
        const ImageRepresentationLocal* _intensityData;
        ImageRepresentationLocal::IntensityHistogramType* _histogram;
    };

// ================================================================================================

    const std::string ImageRepresentationLocal::loggerCat_ = "CAMPVis.core.datastructures.ImageRepresentationLocal";

    ImageRepresentationLocal::ImageRepresentationLocal(const ImageData* parent, WeaklyTypedPointer::BaseType baseType, size_t numChannels)
        : AbstractImageRepresentation(parent)
        , _baseType(baseType)
        , _numChannels(numChannels)
        , _intensityHistogram(0)
    {
        _intensityRangeDirty = true;
    }

    ImageRepresentationLocal::~ImageRepresentationLocal() {
        delete _intensityHistogram;
    }

    const Interval<float>& ImageRepresentationLocal::getNormalizedIntensityRange() const {
        if (_intensityRangeDirty)
            computeNormalizedIntensityRange();

        return _normalizedIntensityRange;
    }

    const ConcurrentGenericHistogramND<float, 1>& ImageRepresentationLocal::getIntensityHistogram() const {
        if (_intensityHistogram == 0)
            computeIntensityHistogram();

        return *_intensityHistogram;
    }

    void ImageRepresentationLocal::computeNormalizedIntensityRange() const {
        tbb::parallel_for(tbb::blocked_range<size_t>(0, getNumElements()), NormalizedIntensityRangeGenerator(this, &_normalizedIntensityRange));
        _intensityRangeDirty = false;
    }

    void ImageRepresentationLocal::computeIntensityHistogram() const {
        delete _intensityHistogram;

        const Interval<float>& i = getNormalizedIntensityRange();
        float mins = i.getLeft();
        float maxs = i.getRight();
        size_t numBuckets = 1024;
        _intensityHistogram = new IntensityHistogramType(&mins, &maxs, &numBuckets);
        tbb::parallel_for(tbb::blocked_range<size_t>(0, getNumElements()), IntensityHistogramGenerator(this, _intensityHistogram));
    }

}