// ================================================================================================
// 
// This file is part of the CAMPVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
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

#include "imagedatalocal.h"

#include "tbb/include/tbb/tbb.h"

namespace campvis {
    
    class IntensityHistogramGenerator {
    public:
        IntensityHistogramGenerator(const ImageDataLocal* intensityData, ImageDataLocal::IntensityHistogramType* histogram)
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
        const ImageDataLocal* _intensityData;
        ImageDataLocal::IntensityHistogramType* _histogram;
    };

// ================================================================================================

    const std::string ImageDataLocal::loggerCat_ = "CAMPVis.core.datastructures.ImageDataLocal";

    ImageDataLocal::ImageDataLocal(size_t dimensionality, const tgt::svec3& size, WeaklyTypedPointer::BaseType baseType, size_t numChannels)
        : ImageData(dimensionality, size)
        , _baseType(baseType)
        , _numChannels(numChannels)
        , _intensityHistogram(0)
    {
    }

    ImageDataLocal::~ImageDataLocal() {
        delete _intensityHistogram;
    }

    const ConcurrentGenericHistogramND<float, 1>& ImageDataLocal::getIntensityHistogram() const {
        if (_intensityHistogram == 0)
            computeIntensityHistogram();

        return *_intensityHistogram;
    }

    void ImageDataLocal::computeIntensityHistogram() const {
        delete _intensityHistogram;

        float mins = 0.f;
        float maxs = 1.f;
        size_t numBuckets = 1024;
        _intensityHistogram = new IntensityHistogramType(&mins, &maxs, &numBuckets);
        tbb::parallel_for(tbb::blocked_range<size_t>(0, getNumElements()), IntensityHistogramGenerator(this, _intensityHistogram));
    }

}