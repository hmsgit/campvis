// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
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

#include "gtest/gtest.h"

#include <sigslot/sigslot.h>

#include <tbb/atomic.h>
#include <tbb/tbb.h>

#include <random>
#include <string>



/**
 * Test class for StringUtils.
 */

class SigslotTest : public ::testing::Test, public sigslot::has_slots {
protected:
    SigslotTest()
        : _randomGenerator(42)
    {
        _countSent0 = 0;
        _countSent1 = 0;
        _countSent2 = 0;
        _countSent3 = 0;
        _countSent4 = 0;
        _countSent5 = 0;
        _countReceived0 = 0;
        _countReceived1 = 0;
        _countReceived2 = 0;
        _countReceived3 = 0;
        _countReceived4 = 0;
        _countReceived5 = 0;
        _isFinished = false;

        s_s0.connect(this, &SigslotTest::onS0);
        s_s1.connect(this, &SigslotTest::onS1);
        s_s2.connect(this, &SigslotTest::onS2);
        s_s3.connect(this, &SigslotTest::onS3);
        s_s4.connect(this, &SigslotTest::onS4);
        s_s5.connect(this, &SigslotTest::onS5);

        s_finished.connect(this, &SigslotTest::onFinished);
    }

    ~SigslotTest() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

    void sendRandomSignal() {
        unsigned int r = (_randomGenerator() - _randomGenerator.min()) % 6;

        switch (r) {
            case 0:
                sendS0();
                break;
            case 1:
                sendS1();
                break;
            case 2:
                sendS2();
                break;
            case 3:
                sendS3();
                break;
            case 4:
                sendS4();
                break;
            case 5:
                sendS5();
                break;
        }
    }


    void sendS0() {
        s_s0.emitSignal();
        _countSent0++;
    }

    void sendS1() {
        s_s1.emitSignal(6);
        _countSent1++;
    }

    void sendS2() {
        s_s2.emitSignal(42, 1337.f);
        _countSent2++;
    }

    void sendS3() {
        s_s3.emitSignal(14, -1.f, "Moin");
        _countSent3++;
    }

    void sendS4() {
        s_s4.emitSignal(-14, 1.f, "Moin", 12.21);
        _countSent4++;
    }
    void sendS5() {
        s_s5.emitSignal(123, 1.f/7.f, "Moin", 10e10, 'a');
        _countSent5++;
    }

    void onS0() {
        _countReceived0++;
    }
    void onS1(int p1) {
        _countReceived1++;
    }
    void onS2(int p1, float p2) {
        _countReceived2++;
    }
    void onS3(int p1, float p2, std::string p3) {
        _countReceived3++;
    }
    void onS4(int p1, float p2, std::string p3, double p4) {
        _countReceived4++;
    }
    void onS5(int p1, float p2, std::string p3, double p4, char p5) {
        _countReceived5++;
        sendS4();
    }

    void onFinished() {
        _isFinished = true;
    }

protected:
    std::default_random_engine _randomGenerator;

    sigslot::signal0 s_s0;
    sigslot::signal1<int> s_s1;
    sigslot::signal2<int, float> s_s2;
    sigslot::signal3<int, float, std::string> s_s3;
    sigslot::signal4<int, float, std::string, double> s_s4;
    sigslot::signal5<int, float, std::string, double, char> s_s5;

    sigslot::signal0 s_finished;
    tbb::atomic<bool> _isFinished;

    tbb::atomic<size_t> _countSent0, _countSent1, _countSent2, _countSent3, _countSent4, _countSent5;
    tbb::atomic<size_t> _countReceived0, _countReceived1, _countReceived2, _countReceived3, _countReceived4, _countReceived5;
};

/** 
 * Tests case related functions.
 * lowercase()
 * uppercase()
 */
TEST_F(SigslotTest, stressTest) {
    const int NUM_SIGNALS = 1000000;

    tbb::parallel_for(tbb::blocked_range<int>(0, NUM_SIGNALS), [&] (const tbb::blocked_range<int>& range) {
        for (int i = range.begin(); i < range.end(); ++i) {
            sendRandomSignal();
        }
    });

    s_finished.emitSignal();
    while (! _isFinished)
        std::this_thread::yield();
    
    EXPECT_EQ(_countSent0, _countReceived0);
    EXPECT_EQ(_countSent1, _countReceived1);
    EXPECT_EQ(_countSent2, _countReceived2);
    EXPECT_EQ(_countSent3, _countReceived3);
    EXPECT_EQ(_countSent4, _countReceived4);
    EXPECT_EQ(_countSent5, _countReceived5);
}

