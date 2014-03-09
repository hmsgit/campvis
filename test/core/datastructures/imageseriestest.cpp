// include gtest library
#include "gtest/gtest.h"

// include(s) for class to be tested
#include "core/datastructures/imageseries.h"

// additional includes
#include "core/datastructures/imagedata.h"


class ImageSeriesTest : public testing::Test {
protected:
    ImageSeriesTest() {
        _image = new campvis::ImageData(2, tgt::svec3(1,2,3), 4);
        _dh = campvis::DataHandle(_image);

        _imgSeries2.addImage(_image);
        _imgSeries3.addImage(_dh);
    }

    ~ImageSeriesTest() {
        //delete _image;
    }

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following two methods
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }


protected:
    campvis::ImageSeries _imgSeries1, _imgSeries2, _imgSeries3;
    campvis::ImageData* _image;
    campvis::DataHandle _dh;

};

TEST_F(ImageSeriesTest, getNumImageTest) {
    EXPECT_EQ(0, _imgSeries1.getNumImages());
    EXPECT_EQ(1, _imgSeries2.getNumImages());
    EXPECT_EQ(1, _imgSeries3.getNumImages());
}

TEST_F(ImageSeriesTest, addImageTest) {
    _imgSeries3.addImage(_image);
    EXPECT_EQ(2, _imgSeries3.getNumImages());
}

TEST_F(ImageSeriesTest, getImageTest) {
    EXPECT_EQ(_dh.getData(), _imgSeries3.getImage(_imgSeries3.getNumImages()-1).getData());
}

TEST_F(ImageSeriesTest, getLocalMemoryFootprintTest) {
    SUCCEED();
}

TEST_F(ImageSeriesTest, getVideoMemoryFootprintTest) {
    SUCCEED();
}

TEST_F(ImageSeriesTest, cloneTest) {
    campvis::ImageSeries* tempSeries = _imgSeries2.clone();
    EXPECT_EQ(_imgSeries2.getNumImages(), tempSeries->getNumImages());
    EXPECT_EQ(_imgSeries2.getLocalMemoryFootprint(), tempSeries->getLocalMemoryFootprint());
    EXPECT_EQ(_imgSeries2.getVideoMemoryFootprint(), tempSeries->getVideoMemoryFootprint());
    EXPECT_EQ(_imgSeries2.getImage(_imgSeries2.getNumImages()-1).getTimestamp(), tempSeries->getImage(tempSeries->getNumImages()-1).getTimestamp());

    delete tempSeries;
}
