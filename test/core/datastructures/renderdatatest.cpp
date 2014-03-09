#include "gtest/gtest.h"

#include "core/datastructures/renderdata.h"

#include "tgt/framebufferobject.h"


class RenderDataTest : public testing::Test {
protected:
    RenderDataTest() {
        //_fbo = new tgt::FramebufferObject();
        //_fbo->attachTexture();
        //_renderData1 = campvis::RenderData(_fbo);
        //_renderData2 = campvis::RenderData(_fbo);
        //_renderData3 = campvis::RenderData(_fbo);
    }

    ~RenderDataTest() {
        //delete _fbo;
    }

    // If the constructor and destructor are not enough for setting up
    // and cleaning up each test, you can define the following two methods
    virtual void SetUp() {
    }

    virtual void TearDown() {
    }


protected:
    //campvis::RenderData _renderData1, _renderData2, _renderData3;
    tgt::FramebufferObject* _fbo;
};

TEST_F(RenderDataTest, getLocalMemoryFootprintTest) {
    SUCCEED();
}
TEST_F(RenderDataTest, getVideoMemoryFootprintTest) {
    SUCCEED();
}

TEST_F(RenderDataTest, cloneTest) {
}

TEST_F(RenderDataTest, getNumColorTexturesTest) {
}

TEST_F(RenderDataTest, getColorTextureTest) {
}

TEST_F(RenderDataTest, getColorDataHandleTest) {
}

TEST_F(RenderDataTest, hasDepthTextureTest) {
}

TEST_F(RenderDataTest, getDepthTextureTest) {
}

TEST_F(RenderDataTest, getDepthDataHandleTest) {
}

TEST_F(RenderDataTest, addColorTextureTest) {
}

TEST_F(RenderDataTest, setDepthTextureTest) {
}

TEST_F(RenderDataTest, bindColorTextureTest) {
}

TEST_F(RenderDataTest, bindDepthTextureTest) {
}

TEST_F(RenderDataTest, bindTest) {
}

