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

#include "core/datastructures/datahandle.h"

#include "core/datastructures/imagedata.h"

/**
 * Test class for DataHandle class. This class along with ImageDataTest and
 * ImageSeriesTest completes the overall test environments for DataHandle and
 * AbstractData.
 */
class DataHandleTest : public testing::Test {
protected:
    DataHandleTest() {
        _data1 = new campvis::ImageData(2, tgt::svec3(1,2,1), 4);

        _dh0 = campvis::DataHandle();
        _dh1 = campvis::DataHandle(_data1);
        _dh2 = campvis::DataHandle(_dh1);
        _dh3 = _dh2;
    }

    ~DataHandleTest() {
        //delete _data1;    // DON'T mess with data added to the DataHandle. More at DataHanlde.h
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }


protected:
    campvis::DataHandle _dh0, _dh1, _dh2, _dh3, _dh4;
    campvis::AbstractData * _data1;

};

TEST_F(DataHandleTest, getDataTest) {
    ASSERT_TRUE(nullptr == _dh0.getData());
    EXPECT_EQ(_dh1.getData(), _dh2.getData());
    EXPECT_EQ(_dh1.getData(), _dh3.getData());

    EXPECT_EQ(nullptr, _dh4.getData());
}

TEST_F(DataHandleTest, getTimestampTest) {
    EXPECT_NE(-1, _dh0.getTimestamp());
    EXPECT_NE(-1, _dh1.getTimestamp());
    EXPECT_NE(-1, _dh2.getTimestamp());
    EXPECT_NE(-1, _dh3.getTimestamp());
    EXPECT_NE(-1, _dh4.getTimestamp());

    EXPECT_EQ(_dh1.getTimestamp(), _dh2.getTimestamp());
    EXPECT_EQ(_dh1.getTimestamp(), _dh3.getTimestamp());
}
