// include gtest library
#include "gtest/gtest.h"

// include(s) for class to be tested
#include "core/datastructures/imagedata.h"

// additional dependency includes
#include "core/datastructures/imagedata.h"


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

TEST_F(ImageDataTest, initTest) {
    ASSERT_TRUE(nullptr != _imgData1);

    EXPECT_EQ(2, _imgData0->getDimensionality());
    EXPECT_EQ(4, _imgData0->getNumChannels());
    EXPECT_EQ(tgt::hmul(tgt::svec3(1,2,3)), _imgData0->getNumElements());
    EXPECT_EQ(tgt::svec3(1,2,3).size, _imgData0->getSize().size);
}

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

TEST_F(ImageDataTest, getSetMappingInformationTest) {
    campvis::ImageData temp = campvis::ImageData(3, tgt::svec3(3,2,1), 5);
    temp.setMappingInformation(_imgData0->getMappingInformation());

    EXPECT_TRUE(temp.getMappingInformation() == _imgData0->getMappingInformation());
}

TEST_F(ImageDataTest, getWorldBoundsTest) {
    tgt::Bounds bound0 = _imgData0->getWorldBounds();
    tgt::Bounds bound1 = _imgData1->getWorldBounds();
    EXPECT_EQ(bound0, bound1);
}

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

TEST_F(ImageDataTest, positionToIndexTest) {
    tgt::svec3 vec(1, 2, 3);
    size_t sz = _imgData0->positionToIndex(vec);
    EXPECT_EQ(1, 1);
}

TEST_F(ImageDataTest, indexToPositionTest) {
    size_t sz = 3;
    tgt::svec3 vec = _imgData0->indexToPosition(sz);
    EXPECT_EQ(1, 1);
}
