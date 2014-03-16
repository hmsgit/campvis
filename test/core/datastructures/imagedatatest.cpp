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
 * Date: March 2014
 */

#include "gtest/gtest.h"

#include "core/datastructures/imagedata.h"

#include "core/datastructures/imagedata.h"

/**
 * Test class for ImageData class.
 */
class ImageDataTest : public testing::Test {
protected:
    ImageDataTest() {
        _imgData0 = new campvis::ImageData(2, tgt::svec3(1,2,3), 4);
        _imgData1 = _imgData0->clone();
        _absData1 = _imgData1;
    }

    ~ImageDataTest() {
        delete _imgData0;
        delete _imgData1;
    }

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following two methods
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }


protected:
    campvis::ImageData *_imgData0, *_imgData1;//, *_imgData2, _imgData3, _imgData4;
    campvis::AbstractData * _absData1;

};

/**
 * Tests the trivial operations of the class.
 * initialization
 * getDimensionality()
 * getNumChannels()
 * getNumElements()
 * getWorldBounds()
 * get/setMappingInformation()
 * positionToIndex()/indexToPosition()
 */
TEST_F(ImageDataTest, miscellaneousTest) {
    ASSERT_TRUE(nullptr != _imgData1);

    EXPECT_EQ(2, _imgData0->getDimensionality());
    EXPECT_EQ(4, _imgData0->getNumChannels());
    EXPECT_EQ(tgt::hmul(tgt::svec3(1,2,3)), _imgData0->getNumElements());
    EXPECT_EQ(tgt::svec3(1,2,3).size, _imgData0->getSize().size);

    tgt::Bounds bound0 = _imgData0->getWorldBounds();
    tgt::Bounds bound1 = _imgData1->getWorldBounds();
    EXPECT_EQ(bound0, bound1);

    campvis::ImageData temp = campvis::ImageData(3, tgt::svec3(3,2,1), 5);
    temp.setMappingInformation(_imgData0->getMappingInformation());

    EXPECT_TRUE(temp.getMappingInformation() == _imgData0->getMappingInformation());

    tgt::svec3 vec(1, 2, 3);
    _imgData0->positionToIndex(vec);
    EXPECT_EQ(1, 1);

    size_t sz = 3;
    _imgData0->indexToPosition(sz);
    EXPECT_EQ(1, 1);
}

/**
 * Tests the clone() function.
 */
TEST_F(ImageDataTest, cloneTest) {
    ASSERT_TRUE(nullptr != _imgData1);

    EXPECT_EQ(_imgData0->getDimensionality(), _imgData1->getDimensionality());
    EXPECT_EQ(_imgData0->getLocalMemoryFootprint(), _imgData1->getLocalMemoryFootprint());
    EXPECT_TRUE(_imgData0->getMappingInformation() == _imgData1->getMappingInformation());
    EXPECT_EQ(_imgData0->getNumChannels(), _imgData1->getNumChannels());
    EXPECT_EQ(_imgData0->getNumElements(), _imgData1->getNumElements());
    //EXPECT_EQ(_imgData0->getRepresentation(), _imgData1->getRepresentation());
    EXPECT_EQ(_imgData0->getSize(), _imgData1->getSize());
    //EXPECT_EQ(_imgData0->getSubImage(), _imgData1->getSubImage());
    EXPECT_EQ(_imgData0->getVideoMemoryFootprint(), _imgData1->getVideoMemoryFootprint());
}

/**
 * Tests getSubImage() method.
 *
 * For test cases 1. whole image and 2. images of size (1,1,1) were checked
 */
TEST_F(ImageDataTest, getSubImageTest) {
    campvis::ImageData *cloned = _imgData0->getSubImage(tgt::svec3(0,0,0), _imgData0->getSize());

    ASSERT_TRUE(nullptr != _imgData1);

    EXPECT_EQ(_imgData0->getDimensionality(), cloned->getDimensionality());
    EXPECT_EQ(_imgData0->getLocalMemoryFootprint(), cloned->getLocalMemoryFootprint());
    EXPECT_TRUE(_imgData0->getMappingInformation() == cloned->getMappingInformation());
    EXPECT_EQ(_imgData0->getNumChannels(), cloned->getNumChannels());
    EXPECT_EQ(_imgData0->getNumElements(), cloned->getNumElements());
    //EXPECT_EQ(_imgData0->getRepresentation(), cloned->getRepresentation());
    EXPECT_EQ(_imgData0->getSize(), cloned->getSize());
    //EXPECT_EQ(_imgData0->getSubImage(), cloned->getSubImage());
    EXPECT_EQ(_imgData0->getVideoMemoryFootprint(), cloned->getVideoMemoryFootprint());

    delete cloned;

    cloned = _imgData0->getSubImage(_imgData0->getSize()-tgt::svec3(1,1,1), _imgData0->getSize());
    EXPECT_EQ(tgt::svec3(1,1,1), cloned->getSize());
}
