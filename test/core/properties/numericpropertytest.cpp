#include "gtest/gtest.h"

#include "core/properties//numericproperty.h"

#include "core/properties/genericproperty.h"
#include "core/properties/abstractproperty.h"
#include "core/pipeline/abstractprocessor.h"

using namespace campvis;

class NumericPropertyTest : public ::testing::Test {
protected:
    NumericPropertyTest() 
        : imax(10), imin(-10), istep(1)
        , _intProp("intProp", "title_intProp", 0, -10, 10, 1, AbstractProcessor::INVALID_RESULT) 
        , _intProp2("intProp2", "title_intProp2", 0, -10, 10, 1, AbstractProcessor::INVALID_RESULT) 
        , dmax(1), dmin(-1), dstep(0.1)
        , _doubleProp("_doubleProp", "title_doubleProp", 0, -1, 1, 0.1, AbstractProcessor::INVALID_RESULT) 
    {
       
    }

    ~NumericPropertyTest() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

protected:
    NumericProperty<int> _intProp;
    NumericProperty<int> _intProp2;
    NumericProperty<double> _doubleProp;

protected:
    int imax, imin, istep;
    double dmax, dmin, dstep;
};

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

/**
 * Other Generic Tests
 */
TEST_F(NumericPropertyTest, getValueTest) {
    EXPECT_EQ(0, this->_intProp.getValue());
}

TEST_F(NumericPropertyTest, setValueTest) {
    this->_intProp.setValue(5);
    EXPECT_EQ(5, this->_intProp.getValue());
}

TEST_F(NumericPropertyTest, addSharedPropertyTest) {
    this->_intProp.addSharedProperty(&this->_intProp2);
    EXPECT_EQ(this->_intProp.getValue(), this->_intProp2.getValue());
    EXPECT_EQ(0, this->_intProp2.getValue());
}

TEST_F(NumericPropertyTest, getSharedPropertiesTest) {
    std::set<AbstractProperty*> sharedProps = this->_intProp.getSharedProperties();
    EXPECT_EQ(0, sharedProps.size());
    this->_intProp.addSharedProperty(&this->_intProp2);
    sharedProps = this->_intProp.getSharedProperties();
    EXPECT_EQ(1, sharedProps.size());
    AbstractProperty *prop = *sharedProps.begin();
    EXPECT_EQ(this->_intProp2.getName(), prop->getName());
}

TEST_F(NumericPropertyTest, removeSharedPropertyTest) {
    this->_intProp.addSharedProperty(&this->_intProp2);
    EXPECT_EQ(this->_intProp.getValue(), this->_intProp2.getValue());
    EXPECT_EQ(0, this->_intProp2.getValue());

    this->_intProp.removeSharedProperty(&this->_intProp2);
    EXPECT_EQ(this->_intProp.getValue(), this->_intProp2.getValue());
    EXPECT_EQ(0, this->_intProp2.getValue());

    this->_intProp.setValue(1);
    EXPECT_EQ(1, this->_intProp.getValue());
    EXPECT_EQ(0, this->_intProp2.getValue());
}


TEST_F(NumericPropertyTest, lockTest) {
    this->_intProp.addSharedProperty(&this->_intProp2);
    this->_intProp.setValue(0);
    EXPECT_EQ(this->_intProp.getValue(), this->_intProp2.getValue());
    EXPECT_EQ(0, this->_intProp2.getValue());

    this->_intProp.lock();

    EXPECT_EQ(this->_intProp.getValue(), this->_intProp2.getValue());
    EXPECT_EQ(0, this->_intProp2.getValue());

    this->_intProp.setValue(1);
    EXPECT_EQ(0, this->_intProp.getValue());
    EXPECT_EQ(0, this->_intProp2.getValue());

}
TEST_F(NumericPropertyTest, unlockTest) {
    this->_intProp.addSharedProperty(&this->_intProp2);
    this->_intProp.setValue(0);
    this->_intProp.lock();

    this->_intProp.setValue(1);
    EXPECT_EQ(0, this->_intProp.getValue());
    EXPECT_EQ(0, this->_intProp2.getValue());

    this->_intProp.unlock();
    EXPECT_EQ(1, this->_intProp.getValue());
    EXPECT_EQ(1, this->_intProp2.getValue());
}
