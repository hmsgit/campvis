#include "gtest/gtest.h"

#include "modules/io/processors/abstractimagereader.h"
#include "modules/io/processors/mhdimagereader.h"
#include "core/properties/genericproperty.h"

TEST(ImageReaderTest, MhdImageReaderTest) {
    campvis::MhdImageReader *ir = new campvis::MhdImageReader();
    ir->setURL(campvis::StringProperty("url","sometitle", "someurl"));
    EXPECT_STRCASEEQ("someurl", ir->p_url.getValue().c_str());
}