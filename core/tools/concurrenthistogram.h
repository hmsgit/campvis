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

#ifndef CONCURRENTHISTOGRAM_H__
#define CONCURRENTHISTOGRAM_H__


#include "cgt/assert.h"
#include "cgt/logmanager.h"
#include "cgt/cgt_math.h"
#include <tbb/atomic.h>

namespace campvis {

    /**
     * Generic implementation of thread-safe n-D histograms.
     * After successful creation ConcurrentGenericHistogramND ensures:
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
         * Returns the number of buckets for the given dimension.
         * \param   dimension   Dimension, must be smaller than ND.
         * \return  _numBuckets[dimension]
         */
        size_t getNumBuckets(size_t dimension) const;

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

        /**
         * Returns the number of elements in the bucket with the most samples.
         * \return  _maxFilling
         */
        size_t getMaxFilling() const { return _maxFilling; };

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
        size_t getArrayIndex(size_t bucketNumbers[ND]) const;

        T _min[ND];                         ///< minimum value for each dimension
        T _max[ND];                         ///< maximum value for each dimension
        size_t _numBuckets[ND];             ///< number of buckets for each dimension
        size_t _arraySize;                  ///< size of _buckets (total number of buckets)
        tbb::atomic<size_t>* _buckets;      ///< array of the buckets storing the histogram
        tbb::atomic<size_t> _numSamples;    ///< total number of sampled elements
        tbb::atomic<size_t> _maxFilling;    ///< number of elements in the bucket with the most samples
    };

// ================================================================================================

    template<typename T, size_t ND>
    campvis::ConcurrentGenericHistogramND<T, ND>::ConcurrentGenericHistogramND(T mins[ND], T maxs[ND], size_t numBuckets[ND]) 
        : _arraySize(1)
        , _buckets(0)
    {
        for (size_t i = 0; i < ND; ++i) {
            _min[i] = mins[i];
            _max[i] = maxs[i];
            cgtAssert(_min[i] < _max[i], "Min must be smaller than Max!");
            _numBuckets[i] = numBuckets[i];
            _arraySize *= _numBuckets[i];
        }

        _buckets = new tbb::atomic<size_t>[_arraySize + 1];
        for (size_t i = 0; i < _arraySize; ++i)
            _buckets[i] = 0;
        _numSamples = 0;
        _maxFilling = 0;
    }

    template<typename T, size_t ND>
    campvis::ConcurrentGenericHistogramND<T, ND>::~ConcurrentGenericHistogramND() {
        delete [] _buckets;
    }

    template<typename T, size_t ND>
    size_t campvis::ConcurrentGenericHistogramND<T, ND>::getNumBuckets(size_t dimension) const {
        cgtAssert(dimension < ND, "Dimension out of bounds.");
        return _numBuckets[dimension];
    }

    template<typename T, size_t ND>
    void campvis::ConcurrentGenericHistogramND<T, ND>::addSample(T sample[ND]) {
        size_t bucketNumbers[ND];
        for (size_t i = 0; i < ND; ++i)
            bucketNumbers[i] = getBucketNumber(i, sample[i]);

        size_t index = getArrayIndex(bucketNumbers);
        ++(_buckets[index]);
        ++_numSamples;

        // thread-safe update of _maxFilling 
        size_t old = 0;
        do {
            old = _maxFilling;
            if (old >= _buckets[index])
                break;
        } while (_maxFilling.compare_and_swap(_buckets[index], old) != old);
    }

    template<typename T, size_t ND>
    size_t campvis::ConcurrentGenericHistogramND<T, ND>::getBucketNumber(size_t dimension, T sample) const {
        cgtAssert(dimension < ND, "Dimension out of bounds.");

        if (sample < _min[dimension] || sample > _max[dimension]) {
            return _numBuckets[dimension];
        }

        double ratio = static_cast<double>(sample - _min[dimension]) / static_cast<double>(_max[dimension] - _min[dimension]);
        int toReturn = static_cast<int>(ratio * _numBuckets[dimension]);
        toReturn = std::max(toReturn, 0);
        toReturn = std::min(toReturn, static_cast<int>(_numBuckets[dimension]) - 1);
        return toReturn;
    }

    template<typename T, size_t ND>
    size_t campvis::ConcurrentGenericHistogramND<T, ND>::getArrayIndex(size_t bucketNumbers[ND]) const {
        size_t index = 0;
        size_t multiplier = 1;
        for (size_t i = 0; i < ND; ++i) {
            if (bucketNumbers[i] == _numBuckets[i])
                return _arraySize;

            index += multiplier * bucketNumbers[i];
            multiplier *= _numBuckets[i];
        }
        return index;
    }


}

#endif // CONCURRENTHISTOGRAM_H__
