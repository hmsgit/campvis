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

#include "core/properties/genericproperty.h"

#include "core/properties/abstractproperty.h"
#include "core/pipeline/abstractprocessor.h"

using namespace campvis;

/**
 * Test class for StringProperty or GenericPropertry<T>. As StringProperty is
 * GenericProperty<std::string>. These tests are also checks the correctness of
 * AbstractProperty class (base class of GenericProperty).
 */
class StringPropertyTest : public ::testing::Test {
protected:
    StringPropertyTest() 
        : _strProp1("prop1", "title_prop1", "value1") 
        , _strProp2("prop2", "title_prop2", "value2") 
    {

    }

    ~StringPropertyTest() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

protected:
    StringProperty _strProp1;
    StringProperty _strProp2;
};

/** 
 * Tests getter and setter of _value
 */ 
TEST_F(StringPropertyTest, getSetValueTest) {
    EXPECT_EQ("value1", this->_strProp1.getValue());

    this->_strProp1.setValue("anotherValue");
    EXPECT_EQ("anotherValue", this->_strProp1.getValue());
}

/**
 * Shared property related tests.
 * Expect b.value == a.value after a.addSharedProperty(b), b shares a's value
 * a.removeSharedProperty(b)  terminates the sharing. b now should have a's value.
 * However, any subsequence change in a doesn't affect b.
 * a.getSharedProperties() returns a Set containing all the shared objects.
 */
TEST_F(StringPropertyTest, sharedPropertyTest) {
    // addSharedProperty()
    this->_strProp1.addSharedProperty(&this->_strProp2);
    EXPECT_EQ(this->_strProp1.getValue(), this->_strProp2.getValue());
    EXPECT_EQ("value1", this->_strProp2.getValue());

    this->_strProp1.setValue("anotherValue");
    EXPECT_EQ("anotherValue", this->_strProp1.getValue());
    EXPECT_EQ("anotherValue", this->_strProp2.getValue());
    this->_strProp2.setValue("someOtherValue");
    EXPECT_EQ("someOtherValue", this->_strProp2.getValue());
    EXPECT_NE("someOtherValue", this->_strProp1.getValue());
    EXPECT_EQ("anotherValue", this->_strProp1.getValue());

    this->_strProp1.setValue("value1");
    // removeSharedProperty()
    this->_strProp1.removeSharedProperty(&this->_strProp2);
    EXPECT_EQ(this->_strProp1.getValue(), this->_strProp2.getValue());
    EXPECT_EQ("value1", this->_strProp2.getValue());

    this->_strProp1.setValue("someValue");
    EXPECT_EQ("someValue", this->_strProp1.getValue());
    EXPECT_EQ("value1", this->_strProp2.getValue());

    // getSharedProperties()
    std::set<AbstractProperty*> sharedProps = this->_strProp1.getSharedProperties();
    EXPECT_EQ(0, sharedProps.size());
    this->_strProp1.addSharedProperty(&this->_strProp2);
    sharedProps = this->_strProp1.getSharedProperties();
    EXPECT_EQ(1, sharedProps.size());
    AbstractProperty *prop = *sharedProps.begin();
    EXPECT_EQ(this->_strProp2.getName(), prop->getName());
}

/**
 * Tests the un/locking mechanism for properties.
 * Once locked any effect of changing value with setValue() is not halted till an
 * unlock action occur. After unlock() the latest value set should be found.
 *
 * \TODO: try thread related stuffs
 */
TEST_F(StringPropertyTest, unlockTest) {
    this->_strProp1.addSharedProperty(&this->_strProp2);
    this->_strProp1.setValue("oldValue");
    EXPECT_EQ(this->_strProp1.getValue(), this->_strProp2.getValue());
    EXPECT_EQ("oldValue", this->_strProp2.getValue());

    this->_strProp1.lock();

    EXPECT_EQ(this->_strProp1.getValue(), this->_strProp2.getValue());
    EXPECT_EQ("oldValue", this->_strProp2.getValue());

    this->_strProp1.setValue("newValue");
    EXPECT_EQ("oldValue", this->_strProp1.getValue());
    EXPECT_EQ("oldValue", this->_strProp2.getValue());

    this->_strProp1.unlock();
    EXPECT_EQ("newValue", this->_strProp1.getValue());
    EXPECT_EQ("newValue", this->_strProp2.getValue());
}
