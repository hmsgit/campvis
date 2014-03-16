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

