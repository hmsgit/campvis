#include "gtest/gtest.h"

#include "core/properties/genericproperty.h"

#include "core/properties/abstractproperty.h"
#include "core/pipeline/abstractprocessor.h"

using namespace campvis;

class StringPropertyTest : public ::testing::Test {
protected:
    StringPropertyTest() 
        : _strProp1("prop1", "title_prop1", "value1", AbstractProcessor::INVALID_RESULT) 
        , _strProp2("prop2", "title_prop2", "value2", AbstractProcessor::INVALID_RESULT) 
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

TEST_F(StringPropertyTest, getValueTest) {
    EXPECT_EQ("value1", this->_strProp1.getValue());
}

TEST_F(StringPropertyTest, setValueTest) {
    this->_strProp1.setValue("anotherValue");
    EXPECT_EQ("anotherValue", this->_strProp1.getValue());
}

TEST_F(StringPropertyTest, addSharedPropertyTest) {
    this->_strProp1.addSharedProperty(&this->_strProp2);
    EXPECT_EQ(this->_strProp1.getValue(), this->_strProp2.getValue());
    EXPECT_EQ("value1", this->_strProp2.getValue());
}

TEST_F(StringPropertyTest, getSharedPropertiesTest) {
    std::set<AbstractProperty*> sharedProps = this->_strProp1.getSharedProperties();
    EXPECT_EQ(0, sharedProps.size());
    this->_strProp1.addSharedProperty(&this->_strProp2);
    sharedProps = this->_strProp1.getSharedProperties();
    EXPECT_EQ(1, sharedProps.size());
    AbstractProperty *prop = *sharedProps.begin();
    EXPECT_EQ(this->_strProp2.getName(), prop->getName());
}

TEST_F(StringPropertyTest, removeSharedPropertyTest) {
    this->_strProp1.addSharedProperty(&this->_strProp2);
    EXPECT_EQ(this->_strProp1.getValue(), this->_strProp2.getValue());
    EXPECT_EQ("value1", this->_strProp2.getValue());

    this->_strProp1.removeSharedProperty(&this->_strProp2);
    EXPECT_EQ(this->_strProp1.getValue(), this->_strProp2.getValue());
    EXPECT_EQ("value1", this->_strProp2.getValue());

    this->_strProp1.setValue("someValue");
    EXPECT_EQ("someValue", this->_strProp1.getValue());
    EXPECT_EQ("value1", this->_strProp2.getValue());
}


TEST_F(StringPropertyTest, lockTest) {
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

}
TEST_F(StringPropertyTest, unlockTest) {
    this->_strProp1.addSharedProperty(&this->_strProp2);
    this->_strProp1.setValue("oldValue");
    this->_strProp1.lock();

    this->_strProp1.setValue("newValue");
    EXPECT_EQ("oldValue", this->_strProp1.getValue());
    EXPECT_EQ("oldValue", this->_strProp2.getValue());

    this->_strProp1.unlock();
    EXPECT_EQ("newValue", this->_strProp1.getValue());
    EXPECT_EQ("newValue", this->_strProp2.getValue());
}
