// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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
    ImageDataTest() 
        : _size(3, 2, 1)
    {
        _imgData0 = new campvis::ImageData(2, _size, 4);
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
    campvis::ImageData *_imgData0, *_imgData1;
    campvis::AbstractData * _absData1;

    tgt::svec3 _size;
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

    EXPECT_EQ(2U, _imgData0->getDimensionality());
    EXPECT_EQ(4U, _imgData0->getNumChannels());
    EXPECT_EQ(tgt::hmul(_size), _imgData0->getNumElements());

    tgt::Bounds bound0 = _imgData0->getWorldBounds();
    tgt::Bounds bound1 = _imgData1->getWorldBounds();
    EXPECT_EQ(bound0, bound1);

    campvis::ImageData temp = campvis::ImageData(3, tgt::svec3(3,2,1), 5);
    temp.setMappingInformation(_imgData0->getMappingInformation());

    EXPECT_TRUE(temp.getMappingInformation() == _imgData0->getMappingInformation());

    for (size_t i = 0; i < _imgData0->getNumElements(); ++i) {
        tgt::svec3 position = _imgData0->indexToPosition(i);
        EXPECT_EQ(_imgData0->positionToIndex(position), i);
    }
    
}

/**
 * Tests the clone() function.
 */
// TODO: extend the test for proper ImageData cloning
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

