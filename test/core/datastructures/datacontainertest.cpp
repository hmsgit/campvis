#include "gtest/gtest.h"

#include "core/datastructures/datacontainer.h"

#include "core/datastructures/datahandle.h"
#include "core/datastructures/imagedata.h"

using namespace campvis;

class DataContainerTest : public ::testing::Test {
protected:
    DataContainerTest() {
        this->_data = new ImageData(2, tgt::svec3(1,2,3), 4);
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

TEST_F(DataContainerTest, addDataTest) {
    EXPECT_EQ(nullptr, this->_dc0->getData("data1").getData());
    this->_dc0->addData("data1", this->_data);
    EXPECT_EQ(this->_data, this->_dc0->getData("data1").getData());
}

TEST_F(DataContainerTest, getDataTest) {
    EXPECT_EQ(nullptr, this->_dc0->getData("data1").getData());
    this->_dc0->addData("data1", this->_data);
    EXPECT_EQ(this->_data, this->_dc0->getData("data1").getData());
}

TEST_F(DataContainerTest, removeDataTest) {
    EXPECT_EQ(nullptr, this->_dc0->getData("data1").getData());
    this->_dc0->addData("data1", this->_data);
    EXPECT_EQ(this->_data, this->_dc0->getData("data1").getData());

    this->_dc0->removeData("data1");
    EXPECT_EQ(nullptr, this->_dc0->getData("data1").getData());
}


TEST_F(DataContainerTest, addDataHandleTest) {
    EXPECT_EQ(nullptr, this->_dc0->getData("data1").getData());
    this->_dc0->addDataHandle("data1", DataHandle(this->_data));
    EXPECT_EQ(this->_data, this->_dc0->getData("data1").getData());
}

TEST_F(DataContainerTest, hasDataTest) {
    EXPECT_EQ(false, this->_dc0->hasData("data1"));
    this->_dc0->addDataHandle("data1", DataHandle(this->_data));
    EXPECT_EQ(true, this->_dc0->hasData("data1"));
    this->_dc0->removeData("data1");
    EXPECT_EQ(false, this->_dc0->hasData("data1"));
}

TEST_F(DataContainerTest, getDataHandlesCopyTest) {
    EXPECT_EQ(0, this->_dc0->getDataHandlesCopy().size());
    this->_dc0->addData("data1", this->_data);
    EXPECT_EQ(1, this->_dc0->getDataHandlesCopy().size());
    std::pair<std::string, DataHandle> pair = this->_dc0->getDataHandlesCopy()[0];
    EXPECT_EQ("data1", pair.first);
    EXPECT_EQ(this->_data, pair.second.getData());
}

TEST_F(DataContainerTest, getNameTest) {
    EXPECT_EQ("dc0", this->_dc0->getName());
}

TEST_F(DataContainerTest, concurrentAccessTest) {
    this->_dc0->addData("data1", this->_data);
    DataHandle dh = this->_dc0->getData("data1");
    AbstractData *someData = new ImageData(2, tgt::svec3(1,2,3), 4);
    this->_dc0->addData("data1", someData);

    EXPECT_EQ(someData, this->_dc0->getData("data1").getData());
    EXPECT_NE(_data, this->_dc0->getData("data1").getData());
    EXPECT_EQ(_data, dh.getData());
}
