// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2015, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
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

#include "core/datastructures/imageseries.h"

#include "core/datastructures/imagedata.h"

/**
 * Test class for ImageSeries class.
 */
class ImageSeriesTest : public testing::Test {
protected:
    ImageSeriesTest() {
        _image = new campvis::ImageData(2, cgt::svec3(1,2,1), 4);
        _dh = campvis::DataHandle(_image);

        _imgSeries2.addImage(_image);
        _imgSeries3.addImage(_dh);
    }

    ~ImageSeriesTest() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }


protected:
    campvis::ImageSeries _imgSeries1, _imgSeries2, _imgSeries3;
    campvis::ImageData* _image;
    campvis::DataHandle _dh;

};

/**
 * Tests trivial functions of the class
 * 
 * addImage()
 * getNumImages()
 * getImage()
 */
TEST_F(ImageSeriesTest, miscellaneousTest) {
    EXPECT_EQ(0U, _imgSeries1.getNumImages());
    EXPECT_EQ(1U, _imgSeries2.getNumImages());
    EXPECT_EQ(1U, _imgSeries3.getNumImages());

    _imgSeries3.addImage(_image);
    EXPECT_EQ(2U, _imgSeries3.getNumImages());

    EXPECT_EQ(_dh.getData(), _imgSeries3.getImage(_imgSeries3.getNumImages()-1).getData());
}


/**
 * Tests the method clone()
 */
TEST_F(ImageSeriesTest, cloneTest) {
    campvis::ImageSeries* tempSeries = _imgSeries2.clone();
    EXPECT_EQ(_imgSeries2.getNumImages(), tempSeries->getNumImages());
    EXPECT_EQ(_imgSeries2.getLocalMemoryFootprint(), tempSeries->getLocalMemoryFootprint());
    EXPECT_EQ(_imgSeries2.getVideoMemoryFootprint(), tempSeries->getVideoMemoryFootprint());
    EXPECT_EQ(_imgSeries2.getImage(_imgSeries2.getNumImages()-1).getTimestamp(), tempSeries->getImage(tempSeries->getNumImages()-1).getTimestamp());

    delete tempSeries;
}
