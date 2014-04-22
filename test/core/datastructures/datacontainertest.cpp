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

#include "core/datastructures/datacontainer.h"

#include "core/datastructures/datahandle.h"
#include "core/datastructures/imagedata.h"

using namespace campvis;

/**
 * Test class for DataContainer class
 */
class DataContainerTest : public ::testing::Test {
protected:
    DataContainerTest() {
        this->_data = new ImageData(2, tgt::svec3(1,2,1), 4);
        this->_dc0 = new DataContainer("dc0");
    }

    ~DataContainerTest() {
        delete this->_dc0;
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

protected:
    DataContainer *_dc0;
    AbstractData * _data;
};

/**
 * Tests the trivial operations of the class.
 * getName()
 * addData()
 * addDataHandle()
 * getData()
 * hasData()
 * removeData()
 */
TEST_F(DataContainerTest, miscellaneousTest) {
    EXPECT_EQ("dc0", this->_dc0->getName());

    EXPECT_EQ(nullptr, this->_dc0->getData("data1").getData());
    this->_dc0->addData("data1", this->_data);
    EXPECT_EQ(this->_data, this->_dc0->getData("data1").getData());
    this->_dc0->addDataHandle("data2", DataHandle(this->_data));

    this->_dc0->removeData("data1");
    EXPECT_EQ(nullptr, this->_dc0->getData("data1").getData());
    // TODO: if the addDataHandle is called here instead of above
    // this->_dc0->addDataHandle("data2", DataHandle(this->_data));
    // the program crashes. Find why!!

    EXPECT_EQ(false, this->_dc0->hasData("data1"));
    EXPECT_EQ(true, this->_dc0->hasData("data2"));
}

/**
 * Tests the function getDataHandlesCopy.
 */
TEST_F(DataContainerTest, getDataHandlesCopyTest) {
    EXPECT_EQ(0, this->_dc0->getDataHandlesCopy().size());
    this->_dc0->addData("data1", this->_data);
    EXPECT_EQ(1, this->_dc0->getDataHandlesCopy().size());
    std::pair<std::string, DataHandle> pair = this->_dc0->getDataHandlesCopy()[0];
    EXPECT_EQ("data1", pair.first);
    EXPECT_EQ(this->_data, pair.second.getData());
}

/**
 * Tests the concurrent access settings of the class.
 *
 * DataHandle of previously added data obtained with getData() should remains valid
 * even if the data in the container is replaced with new data.
 */
TEST_F(DataContainerTest, concurrentAccessTest) {
    this->_dc0->addData("data1", this->_data);
    DataHandle dh = this->_dc0->getData("data1");
    AbstractData *someData = new ImageData(2, tgt::svec3(1,2,1), 4);
    this->_dc0->addData("data1", someData);

    EXPECT_EQ(someData, this->_dc0->getData("data1").getData());
    EXPECT_NE(_data, this->_dc0->getData("data1").getData());
    EXPECT_EQ(_data, dh.getData());
}
