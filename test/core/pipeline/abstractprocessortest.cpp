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

#include "core/pipeline/abstractprocessor.h"

#include "core/datastructures/datacontainer.h"
#include "core/datastructures/imagedata.h"

using namespace campvis;


// FIXME: This is not what I was supposed to be. fix me please. 
class DummyTestProcessor : public AbstractProcessor {
public:
    DummyTestProcessor () {
        _invalidateExternally = false;
        this->invalidate(AbstractProcessor::VALID);
    }
    ~DummyTestProcessor () {}

    virtual const std::string getName() const { return "DummyTestProcessor"; };
    virtual const std::string getDescription() const { return "A dummy processor for the testing purposes only."; };
    virtual const std::string getAuthor() const { return "Hossain Mahmud <mahmud@in.tum.de>"; };
    virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };
    
    virtual void updateResult(DataContainer& dataContainer) {
        dataContainer.removeData("ImageData");
        dataContainer.addData("ImageData", new ImageData(2, tgt::svec3(1,2,1), 4));
    }

    void setExternalInvalidation(bool status, AbstractProcessor::InvalidationLevel level) {
        _invalidateExternally = status;
        this->invalidate(level);
        
        this->invalidate(this->getInvalidationLevel());
    }

private:
    bool _invalidateExternally;
};


/**
 * Test class for AbstractProcessor. Instead of testing any implemented processor, we tested 
 * the functionality with a dummy test class.
 */
class AbstractProcessorTest : public ::testing::Test {
protected:
    AbstractProcessorTest() 
        : _dataContainer("testContainer")
    {
        this->_dataContainer.addData("ImageData", new ImageData(2, tgt::svec3(1,2,1), 4));
    }

    ~AbstractProcessorTest() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

protected:
    DataContainer _dataContainer;
    DummyTestProcessor _processor1, _processor2;
};

/** 
 * Tests invalidation of data
 */ 
TEST_F(AbstractProcessorTest, invalidationTest) {
    this->_processor1.process(this->_dataContainer);
    EXPECT_EQ(AbstractProcessor::VALID, this->_processor1.getInvalidationLevel());
    
    this->_processor1.setExternalInvalidation(true, AbstractProcessor::INVALID_RESULT);
    this->_processor1.process(this->_dataContainer);
    EXPECT_NE(AbstractProcessor::VALID, this->_processor1.getInvalidationLevel());
}
