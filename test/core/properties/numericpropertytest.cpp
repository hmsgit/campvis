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

#include "core/properties//numericproperty.h"

#include "core/properties/genericproperty.h"
#include "core/properties/abstractproperty.h"
#include "core/pipeline/abstractprocessor.h"

using namespace campvis;


/**
 * Test class for NumericProperty. The units from GenericProperty and AbstractProperty are
 * are being tested at StringPropertyTest. Here are the additional max/min related tests. 
 */

class NumericPropertyTest : public ::testing::Test {
protected:
    NumericPropertyTest() 
        : imax(10), imin(-10), istep(1)
        , _intProp("intProp", "title_intProp", 0, -10, 10, 1) 
        , _intProp2("intProp2", "title_intProp2", 0, -10, 10, 1) 
        , dmax(1), dmin(-1), dstep(0.1)
        , _doubleProp("_doubleProp", "title_doubleProp", 0, -1, 1, 0.1) 
    {
       
    }

    ~NumericPropertyTest() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

protected:
    int imax, imin, istep;
    NumericProperty<int> _intProp;
    NumericProperty<int> _intProp2;
    double dmax, dmin, dstep;
    NumericProperty<double> _doubleProp;

};

/** 
 * Checks whether the value get clamped at the max limit.
 */
TEST_F(NumericPropertyTest, maxTest) {
    this->_intProp.setValue(imax);
    EXPECT_EQ(imax, this->_intProp.getValue());
    this->_intProp.setValue(imax + 1);
    EXPECT_EQ(imax, this->_intProp.getValue());

    this->_doubleProp.setValue(dmax);
    EXPECT_DOUBLE_EQ(dmax, this->_doubleProp.getValue());
    this->_doubleProp.setValue(dmax + 0.01);
    EXPECT_DOUBLE_EQ(dmax, this->_doubleProp.getValue());
}

/** 
 * Checks whether the value get clamped at the min limit.
 */
TEST_F(NumericPropertyTest, minTest) {
    this->_intProp.setValue(imin);
    EXPECT_EQ(imin, this->_intProp.getValue());
    this->_intProp.setValue(imin - 1);
    EXPECT_EQ(imin, this->_intProp.getValue());

    this->_doubleProp.setValue(dmin);
    EXPECT_DOUBLE_EQ(dmin, this->_doubleProp.getValue());
    this->_doubleProp.setValue(dmin - 0.01);
    EXPECT_DOUBLE_EQ(dmin, this->_doubleProp.getValue());
}

/** 
 * Checks whether the value is clamped at the max limit. It should.
 */
TEST_F(NumericPropertyTest, increaseTest) {
    EXPECT_EQ(0, this->_intProp.getValue());
    this->_intProp.increment();
    EXPECT_EQ(istep, this->_intProp.getValue());

    this->_intProp.setValue(imax);
    EXPECT_EQ(imax, this->_intProp.getValue());
    this->_intProp.increment();
    EXPECT_EQ(imax, this->_intProp.getValue());

    EXPECT_EQ(0, this->_doubleProp.getValue());
    this->_doubleProp.increment();
    EXPECT_EQ(dstep, this->_doubleProp.getValue());

    this->_doubleProp.setValue(dmax);
    EXPECT_EQ(dmax, this->_doubleProp.getValue());
    this->_doubleProp.increment();
    EXPECT_EQ(dmax, this->_doubleProp.getValue());
}


/** 
 * Checks whether the value is clamped at the min limit. It should.
 */
TEST_F(NumericPropertyTest, decreaseTest) {
    EXPECT_EQ(0, this->_intProp.getValue());
    this->_intProp.decrement();
    EXPECT_EQ(-istep, this->_intProp.getValue());

    this->_intProp.setValue(imin);
    EXPECT_EQ(imin, this->_intProp.getValue());
    this->_intProp.decrement();
    EXPECT_EQ(imin, this->_intProp.getValue());

    EXPECT_EQ(0, this->_doubleProp.getValue());
    this->_doubleProp.decrement();
    EXPECT_EQ(-dstep, this->_doubleProp.getValue());

    this->_doubleProp.setValue(dmin);
    EXPECT_EQ(dmin, this->_doubleProp.getValue());
    this->_doubleProp.decrement();
    EXPECT_EQ(dmin, this->_doubleProp.getValue());
}