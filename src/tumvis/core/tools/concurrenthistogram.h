
#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/tgt_math.h"
#include "tbb/include/tbb/atomic.h"

namespace TUMVis {

    /**
     * Generic implementation of thread-safe n-D histograms.
     * After successfull creation ConcurrentGenericHistogramND ensures:
     *  * Calling addSample() is thread-safe.
     * 
     * \tparam  T   Base data type of the histogram elements
     * \tparam  ND  Dimensionality of the histogram
     */
    template<typename T, size_t ND>
    class ConcurrentGenericHistogramND {
    public:
        /**
         * Creates a new ND-histogram with the given bounds and number of buckets.
         * \param   mins        Array of mininum values for each dimension (must have at least ND elements)
         * \param   maxs        Array of maximum values for each dimension (must have at least ND elements)
         * \param   numBuckets  Array of number of buckets for each dimension (must have at least ND elements)
         */
        ConcurrentGenericHistogramND(T mins[ND], T maxs[ND], size_t numBuckets[ND]);

        /**
         * Destructor
         */
        virtual ~ConcurrentGenericHistogramND();

        /**
         * Adds the given sample to the histogram.
         * \note    This method is thread-safe.
         * \param   sample  Sample to add, array of the values for each dimension (must have at least ND elements)
         */
        void addSample(T sample[ND]);

        /**
         * Returns a const pointer to the raw data.
         * \return  _buckets
         */
        const tbb::atomic<size_t>* getBuckets() const { return _buckets; };

        /**
         * Returns the number of elements of the bucket with the given index.
         * \param   index   Array index of the bucket to return.
         * \return  _buckets[index]
         */
        size_t getNumElements(size_t index) const { return _buckets[index]; };

        /**
         * Returns the number of elements in the given bucket.
         * \param   bucket[ND]  Array of the bucket number for each dimension (must have at least ND elements)
         * \return  The number of elements in the given bucket.
         */
        size_t getNumElements(size_t bucket[ND]);

        /**
         * Get the total number of samples in this histogram.
         * \return  _numSamples
         */
        size_t getNumSamples() const { return _numSamples; };

    protected:
        /**
         * Transforms the sample value for the given dimension into the corresponding bucket number.
         * \param   dimension   Dimension
         * \param   sample      Sample value to transform
         * \return  The bucket number for \a sample in the given dimension.
         */
        size_t getBucketNumber(size_t dimension, T sample) const;

        /**
         * Transforms the array of bucket numbers into the corresponding array index in _buckets.
         * \param   bucketNumbers   Array of the bucket number for each dimension (must have at least ND elements).
         * \return  The array index for the given bucket numbers.
         */
        size_t getArrayIndex(size_t* bucketNumbers) const;

        T _min[ND];                         ///< minimum value for each dimension
        T _max[ND];                         ///< maximum value for each dimension
        size_t _numBuckets[ND];             ///< number of buckets for each dimension
        size_t _arraySize;                  ///< size of _buckets (total number of buckets)
        tbb::atomic<size_t>* _buckets;      ///< array of the buckets storing the histogram
        tbb::atomic<size_t> _numSamples;    ///< total number of sampled elements
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
    void TUMVis::ConcurrentGenericHistogramND<T, ND>::addSample(T sample[ND]) {
        size_t bucketNumbers[ND];
        for(int i = 0; i < ND; ++i)
            bucketNumbers[i] = getBucketNumber(i, sample[i]);

        size_t index = getArrayIndex(bucketNumbers);
        ++(_buckets[index]);
        ++_numSamples;
    }


    template<typename T, size_t ND>
    size_t TUMVis::ConcurrentGenericHistogramND<T, ND>::getBucketNumber(size_t dimension, T sample) const {
        tgtAssert(dimension < ND, "Dimension out of bounds.");
#ifdef TUMVIS_DEBUG
        if (sample < _min[dimension] || sample > _max[dimension])
            LWARNINGC("TUMVis.core.tools.ConcurrentGenericHistogramND", "Added sample " << sample << " out of bounds for dimension" << dimension << ".");
#endif
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