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

/**
 * Author: Hossain Mahmud <mahmud@in.tum.de>
 * Date: April 2014
 */

#include "gtest/gtest.h"

#include "core/tools/concurrenthistogram.h"

#include <tbb/tbb.h>

using namespace campvis;


/**
 * Test class for ConcurrentGenericHistogramND. Initializes a uniform distribution
 * over ND dimensional space. Buckets are chosen in each unit, i.e. number of buckets
 * is range+1. 
 */

template<size_t ND>
class ConcurrentHistogramND {
protected:
    ConcurrentHistogramND()
    {
        max = new int[ND];
        min = new int[ND];
        numBuckets = new size_t[ND];
        for (int i = 0; i < ND; i++) { min[i] = 0; }
        for (int i = 0; i < ND; i++) { max[i] = 100; }
        for (int i = 0; i < ND; i++) { 
            numBuckets[i] = max[i] - min[i] + 1; 
        }

        numSamples = static_cast<int>(pow(max[0]-min[0]+1, ND));
        initSamples();

        histogram = new int[static_cast<int>(pow(max[0]-min[0]+1, ND))];
        computeHistogram();

        _cgh = new campvis::ConcurrentGenericHistogramND<int, ND>(min, max, numBuckets);
    }

    ~ConcurrentHistogramND() {
        delete max;
        delete min;
        delete numBuckets;
        delete histogram;
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

    void initSamples() {
        int range = max[0] - min[0] + 1;
        for(int i = 0; i < numSamples; i++) {
            std::vector<int> sample;
            int x = i;
            for(int j = ND-1; j >= 0; j--) {
                sample.push_back(x / static_cast<int>(pow(range, j)));
                x = x % static_cast<int>(pow(range, j));
            }
            samples.push_back(sample);
        }
    }
    void computeHistogram() {
        for (int i = 0; i < numSamples; i++) {
            histogram[i] = 1;
        }
    }

    int getND() {return ND;}

protected:
    int *max, *min;
    size_t *numBuckets;
    campvis::ConcurrentGenericHistogramND<int, ND>* _cgh;

    int numSamples;
    std::vector<std::vector<int> > samples;
    int *histogram;

};

/** 
 * Test class 1D system.
 */
class ConcurrentHistogram1DTest : public ConcurrentHistogramND<1>, public ::testing::Test {
public:
    ConcurrentHistogram1DTest() {
    }
};

/** 
 * Expected number of elements at each bucket should be 1
 */
TEST_F(ConcurrentHistogram1DTest, concurrentAddSampleTest) {
    tbb::parallel_for(tbb::blocked_range<size_t>(0, samples.size()), [&] (const tbb::blocked_range<size_t>& range) {
        for (size_t i = range.begin(); i != range.end(); ++i) {
            std::vector<int> vec = samples[i];
            int value[10];
            for (int k = 0; k < getND(); k++) {
                value[k] = vec[k];
            }
            _cgh->addSample(&(samples[i].front()));
        }
    });

    for (int i = 0; i < getND(); i++) {
        for(int j = 0; j < numBuckets[i]; j ++) {
            EXPECT_EQ(histogram[i], _cgh->getNumElements(j));
        }
    }
}

/** 
 * Test class 2D system.
 */
class ConcurrentHistogram2DTest : public ConcurrentHistogramND<2>, public ::testing::Test {
public:
    ConcurrentHistogram2DTest() {
    }
};

/** 
 * Expected number of elements at each bucket should be 1
 */
TEST_F(ConcurrentHistogram2DTest, concurrentAddSampleTest) {
    tbb::parallel_for(tbb::blocked_range<size_t>(0, samples.size()), [&] (const tbb::blocked_range<size_t>& range) {
        for (size_t i = range.begin(); i != range.end(); ++i) {
            std::vector<int> vec = samples[i];
            int value[10];
            for (int k = 0; k < getND(); k++) {
                value[k] = vec[k];
            }
            _cgh->addSample(&(samples[i].front()));
        }
    });

    for (int i = 0; i < getND(); i++) {
        for(int j = 0; j < numBuckets[i]; j ++) {
            EXPECT_EQ(histogram[i], _cgh->getNumElements(j));
        }
        break;
    }
}