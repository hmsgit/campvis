
#include "tgt/tgt_math.h"
#include "tbb/include/tbb/atomic.h"

namespace TUMVis {

    template<typename T, size_t ND>
    class ConcurrentGenericHistogramND {
    public:
        ConcurrentGenericHistogramND(T mins[ND], T maxs[ND], size_t numBuckets[ND]);

        virtual ~ConcurrentGenericHistogramND();

        void addSample(T sample1[ND]);

        const tbb::atomic<size_t>* getBuckets() const { return _buckets; };

        size_t getNumSamples() const { return _numSamples; };

    protected:
        size_t getBucketNumber(size_t dimension, T sample) const;

        size_t getArrayIndex(size_t* bucketNumbers) const;

        T _min[ND];
        T _max[ND];
        size_t _numBuckets[ND];
        size_t _arraySize;
        tbb::atomic<size_t>* _buckets;
        tbb::atomic<size_t> _numSamples;
    };

// ================================================================================================

    template<typename T, size_t ND>
    TUMVis::ConcurrentGenericHistogramND<T, ND>::ConcurrentGenericHistogramND(T mins[ND], T maxs[ND], size_t numBuckets[ND]) 
        : _arraySize(1)
        , _buckets(0)
    {
        for (size_t i = 0; i < ND; ++i) {
            _min[i] = mins[i];
            _max[i] = maxs[i];
            tgtAssert(_min[i] < _max[i], "Min must be smaller than Max!");
            _numBuckets[i] = numBuckets[i];
            _arraySize *= _numBuckets[i];
        }

        _buckets = new tbb::atomic<size_t>[_arraySize];
        for (size_t i = 0; i < _arraySize; ++i)
            _buckets[i] = 0;
    }

    template<typename T, size_t ND>
    TUMVis::ConcurrentGenericHistogramND<T, ND>::~ConcurrentGenericHistogramND() {
        delete [] _buckets;
    }

    template<typename T, size_t ND>
    void TUMVis::ConcurrentGenericHistogramND<T, ND>::addSample(T sample1[ND]) {
        size_t bucketNumbers[ND];
        for(int i = 0; i < ND; ++i)
            bucketNumbers[i] = getBucketNumber(i, sample1[i]);

        size_t index = getArrayIndex(bucketNumbers);
        ++(_buckets[index]);
        ++_numSamples;
    }


    template<typename T, size_t ND>
    size_t TUMVis::ConcurrentGenericHistogramND<T, ND>::getBucketNumber(size_t dimension, T sample) const {
        double ratio = static_cast<double>(sample - _min[dimension]) / static_cast<double>(_max[dimension] - _min[dimension]);
        return static_cast<size_t>(tgt::clamp(static_cast<int>(ratio * _numBuckets[dimension]), static_cast<int>(0), static_cast<int>(_numBuckets[dimension] - 1)));
    }

    template<typename T, size_t ND>
    size_t TUMVis::ConcurrentGenericHistogramND<T, ND>::getArrayIndex(size_t* bucketNumbers) const {
        size_t index = 0;
        size_t multiplier = 1;
        for (size_t i = 0; i < ND; ++i) {
            index += multiplier * bucketNumbers[i];
            multiplier *= _numBuckets[i];
        }
        return index;
    }


}
