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

#include "core/tools/glreduction.h"

#include "core/datastructures/imagedata.h"
#include "core/datastructures/imagerepresentationlocal.h"

#include "tgt/framebufferobject.h"

using namespace campvis;


/**
 * Test class for GlReduction. Creates an ImageData with a local representation
 * and known data. Then compares the output of GlReduction::reduce() with 
 * expected values.
 */
class GlReductionTest : public ::testing::TestWithParam<int> {
protected:
    GlReductionTest() {
        name = new tgt::FramebufferObject();

        width = 10;
        height = 10;
        
        EXPECT_TRUE(GetParam() == 2 || GetParam() == 3);
        depth = (GetParam() == 3)? 10 : 1;

        rawData = new float[width * height * depth];
        initData(rawData, width * height * depth);
        imgData = new ImageData(depth == 1? 2 : 3, tgt::svec3(width, height, depth), 1);
        localRep = ImageRepresentationLocal::create(imgData, WeaklyTypedPointer(WeaklyTypedPointer::FLOAT, 1, rawData));
        glReduction = nullptr;
    }

    ~GlReductionTest() {
        delete imgData;
        delete glReduction;
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }
    
    void initData(float* data, int size) {
        for (int i = 0; i < size; i++ ) {
            data[i] = 1;
        }
        srand(static_cast<unsigned>(time(NULL)));
        data[rand() % size] = 0;
    }
    
    int getSize() {
        return width * height * depth;
    }

protected:
    tgt::FramebufferObject *name;

    int width, height, depth;
    float* rawData;
    ImageData* imgData;
    ImageRepresentationLocal *localRep;

    GlReduction* glReduction;
};

/** 
 * Checks whether the OpenGL context is valid here.
 */
TEST_P(GlReductionTest, isScopedLockWorking) {
    if (name->getId() != 0){
        SUCCEED();
    } else {
        FAIL();
    }
}

/** 
 * Checks whether the local representation is valid.
 */
TEST_P(GlReductionTest, localRepCreationTest) {
    EXPECT_NE(nullptr, localRep);
}

/** 
 * Tests for ReducationOperator::MIN.
 */
TEST_P(GlReductionTest, minTest) {
    glReduction = new GlReduction(GlReduction::MIN);
    std::vector<float> reduced = glReduction->reduce(imgData);

    ASSERT_NEAR(0, reduced[0], 0.0001);
    ASSERT_NEAR(0, reduced[1], 0.0001);
}

/** 
 * Tests for ReducationOperator::MAX.
 */
TEST_P(GlReductionTest, maxTest) {
    glReduction = new GlReduction(GlReduction::MAX);
    std::vector<float> reduced = glReduction->reduce(imgData);

    ASSERT_NEAR(1, reduced[0], 0.0001);
    ASSERT_NEAR(0, reduced[1], 0.0001);
}

/** 
 * Tests for ReducationOperator::PLUS.
 */
TEST_P(GlReductionTest, sumTest) {
    glReduction = new GlReduction(GlReduction::PLUS);
    std::vector<float> reduced = glReduction->reduce(imgData);

    ASSERT_NEAR(getSize() - 1, reduced[0], 0.0001);
    ASSERT_NEAR(0, reduced[1], 0.0001);
}

/** 
 * Tests for ReducationOperator::MULTIPLICATION.
 */
TEST_P(GlReductionTest, multTest) {
    glReduction = new GlReduction(GlReduction::MULTIPLICATION);
    std::vector<float> reduced = glReduction->reduce(imgData);

    ASSERT_NEAR(0, reduced[0], 0.0001);
    ASSERT_NEAR(0, reduced[1], 0.0001);
}

INSTANTIATE_TEST_CASE_P(2DTest, GlReductionTest, ::testing::Values(2));
INSTANTIATE_TEST_CASE_P(3DTest, GlReductionTest, ::testing::Values(3));

