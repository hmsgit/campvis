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
#include <thread>

using namespace campvis;

class DummyTestProcessor : public AbstractProcessor {
public:
    DummyTestProcessor () {}
    ~DummyTestProcessor () {}

    /// \see AbstractProcessor::getName()
    virtual const std::string getName() const { return "DummyTestProcessor"; };
    /// \see AbstractProcessor::getDescription()
    virtual const std::string getDescription() const { return "A dummy processor for the testing purposes only."; };
    /// \see AbstractProcessor::getAuthor()
    virtual const std::string getAuthor() const { return "Hossain Mahmud <mahmud@in.tum.de>"; };
    /// \see AbstractProcessor::getProcessorState()
    virtual ProcessorState getProcessorState() const { return AbstractProcessor::TESTING; };
    /// \see AbstractProcessor::updateResult()
    virtual void updateResult(DataContainer& dataContainer) {
        dataContainer.removeData("ImageData");
        dataContainer.addData("ImageData", new ImageData(2, tgt::svec3(1,2,3), 4));
    }
    
    void lockProcessor() {
        AbstractProcessor::lockProcessor();
    }
    void unlockProcessor() {
        AbstractProcessor::unlockProcessor();
    }
    //void process(DataContainer& data, bool unlockInExtraThread=false) {
    //    AbstractProcessor::process(data, unlockInExtraThread);
    //    std::this_thread::yield();
    //}
private:

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
        this->_dataContainer.addData("ImageData", new ImageData(2, tgt::svec3(1,2,3), 4));
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

void callProcess(DummyTestProcessor& processor, DataContainer& dataContainer) {
    printf("entering cprocess\n");
    processor.process(dataContainer);
    printf("going to bed\n");
    std::this_thread::sleep_for(std::chrono::microseconds(1000));
    printf("leaving cprocess\n");
}

void callInvalidate(DummyTestProcessor& processor, AbstractProcessor::InvalidationLevel level) {
    printf("entering cinvalidate\n");
    processor.invalidate(level);
    printf("leaving cinvalidate\n");
}

/** 
 * Tests invalidation of data
 */ 
TEST_F(AbstractProcessorTest, invalidationTest) {
    this->_processor1.process(this->_dataContainer);
    EXPECT_EQ(AbstractProcessor::InvalidationLevel::VALID, this->_processor1.getInvalidationLevel());
    
    //this->_processor1.lockProcessor();    //this doesn't work. AbstractProcessor::lockProcessor():while doesn't end
    this->_processor1.process(this->_dataContainer);
    EXPECT_EQ(AbstractProcessor::InvalidationLevel::VALID, this->_processor1.getInvalidationLevel());

    std::thread first(callProcess, this->_processor1, this->_dataContainer);
    std::thread second(callInvalidate, this->_processor1, AbstractProcessor::InvalidationLevel::INVALID_RESULT);
    first.join();
    second.join();
    EXPECT_EQ(AbstractProcessor::InvalidationLevel::VALID, this->_processor1.getInvalidationLevel());

}

/**
Look, there's no easy way to do this. I'm working on a project that is 
inherently multi threaded. Events come in from the operating system and 
I have to process them concurrently.

The simplest way to deal with testing complex, multi threaded application 
code is this: If its too complex to test, you're doing it wrong. If you 
have a single instance that has multiple threads acting upon it, and you 
can't test situations where these threads step all over each other, then 
your design needs to be redone. Its both as simple and as complex as this.

There are many ways to program for multi threading that avoids threads 
running through instances at the same time. The simplest is to make all 
your objects immutable. Of course, that's not usually possible. So you 
have to identify those places in your design where threads interact 
with the same instance and reduce the number of those places. By doing 
this, you isolate a few classes where multi threading actually occurs, 
reducing the overall complexity of testing your system.

But you have to realize that even by doing this you still can't test 
every situation where two threads step on each other. To do that, you'd 
have to run two threads concurrently in the same test, then control 
exactly what lines they are executing at any given moment. The best you 
can do is simulate this situation. But this might require you to code 
specifically for testing, and that's at best a half step towards a true 
solution.

Probably the best way to test code for threading issues is through 
static analysis of the code. If your threaded code doesn't follow a 
finite set of thread safe patterns, then you might have a problem. I 
believe Code Analysis in VS does contain some knowledge of threading, 
but probably not much.

Look, as things stand currently (and probably will stand for a good time 
to come), the best way to test multi threaded apps is to reduce the 
complexity of threaded code as much as possible. Minimize areas where 
threads interact, test as best as possible, and use code analysis to 
identify danger areas.
*/