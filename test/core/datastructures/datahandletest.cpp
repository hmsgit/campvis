#include "gtest/gtest.h"

#include "core/datastructures/datahandle.h"

#include "core/datastructures/imagedata.h"


class DataHandleTest : public testing::Test {
protected:
    DataHandleTest() {
        _data1 = new campvis::ImageData(2, tgt::svec3(1,2,3), 4);

        _dh0 = campvis::DataHandle();
        _dh1 = campvis::DataHandle(_data1);
        _dh2 = campvis::DataHandle(_dh1);
        _dh3 = _dh2;
    }

    ~DataHandleTest() {
        //delete _data1;
    }

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following two methods
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
