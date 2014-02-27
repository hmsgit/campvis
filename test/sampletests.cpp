#include "gtest/gtest.h"

//#include "someclass.h"

//#include "modules/io/processors/abstractimagereader.h"
//#include "modules/io/processors/mhdimagereader.h"
//#include "core/properties/genericproperty.h"

class SomeClassTest : public ::testing::Test {
protected:
    SomeClassTest() {

    }

    ~SomeClassTest() {

    }

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following two methods
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

private:


};

TEST_F(SomeClassTest, assertTrue) {
    ASSERT_EQ(1, 1);
}

TEST_F(SomeClassTest, assertFalse) {
    ASSERT_NE(1, 0);
}


//TEST(ImageReaderTest, MhdImageReaderTest) {
//    campvis::MhdImageReader *ir = new campvis::MhdImageReader();
//    ir->setURL(campvis::StringProperty("url","sometitle", "someurl"));
//    EXPECT_STRCASEEQ("someurl", ir->p_url.getValue().c_str());
//}