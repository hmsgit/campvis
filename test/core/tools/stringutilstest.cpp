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

/**
 * Author: Hossain Mahmud <mahmud@in.tum.de>
 * Date: March 2014
 */

#include "gtest/gtest.h"

#include "core/tools/stringutils.h"

using namespace campvis;


/**
 * Test class for StringUtils.
 */

class StringUtilTest : public ::testing::Test {
protected:
    StringUtilTest() {
    }

    ~StringUtilTest() {
    }

    virtual void SetUp() {
    }

    virtual void TearDown() {
    }

protected:
    std::string _str;
};

/** 
 * Tests case related functions.
 * lowercase()
 * uppercase()
 */
TEST_F(StringUtilTest, caseOperationsTest) {
    EXPECT_EQ("UPPER CASE STRING", StringUtils::uppercase("upper Case string"));
    EXPECT_EQ("lower case string", StringUtils::lowercase("LOWER Case string"));
    EXPECT_EQ("+-*/@#$%^&*0=", StringUtils::uppercase("+-*/@#$%^&*0="));
    EXPECT_EQ("+-*/@#$%^&*0=", StringUtils::lowercase("+-*/@#$%^&*0="));
    EXPECT_EQ("102.3", StringUtils::uppercase("102.3"));
    EXPECT_EQ("102.3", StringUtils::lowercase("102.3"));
}

/** 
 * Tests parseFloats().
 * 
 */
TEST_F(StringUtilTest, parseFloatsTest) {
    std::vector<std::string> parsed = StringUtils::parseFloats(
        "-1m-0.0 xyz +.12  123.zx0 --0.14 xyz");
    static const float value[] = 
        {-1, -0.0f,    .12f,123.0f,0.0f};
    std::vector<float> expected (value, value + sizeof(value) / sizeof(float) );
    //std::vector<float> expected = {0, .12, 123, 0}; // You may use this if your compiler supports, C++11
    
    EXPECT_EQ(expected.size(), parsed.size());
    for (size_t i = 0; i < expected.size(); i++ ) {
        EXPECT_FLOAT_EQ(expected[i], StringUtils::fromString<float>(parsed[i].c_str()));
    }
}

/** 
 * Tests split*()s.
 * 
 */
TEST_F(StringUtilTest, splitTest) {
    std::vector<std::string> parsed = StringUtils::split(
        "one two three", " ");
    std::string str[] = {"one", "two", "three", "^_^"}; // "^_^" is used to indicate last token 
    std::vector<std::string> expected;

    for (int i= 0; "^_^" != str[i] ; i++) {
        expected.push_back(str[i]);
    }
    
    EXPECT_EQ(expected.size(), parsed.size());
    for (size_t i = 0; i < expected.size(); i++ ) {
        EXPECT_EQ(expected[i], parsed[i]);
    }

    //splitStringsafe()
    parsed.clear();
    expected.clear();

    parsed = StringUtils::splitStringsafe("one two three", " ", '|');
    parsed = StringUtils::splitStringsafe("one 'two three' 'four fi\'\'ve' six \' \' ", " ", '\'');
    std::string str2[] = { "one", "two three", "four fi\'ve", "six", "" , "^_^"};//{"one", "two", "three", "^_^"};    // "^_^" is used to indicate last token 

    for (int i= 0; "^_^" != str2[i] ; i++) {
        expected.push_back(str2[i]);
    }

    EXPECT_EQ(expected.size(), parsed.size());
    for (size_t i = 0; i < expected.size(); i++ ) {
        EXPECT_EQ(expected[i], parsed[i]);
    }
}

/** 
 * Tests replace()
 * 
 */
TEST_F(StringUtilTest, replaceTest) {
    EXPECT_EQ("REPLACEDsomestringREPLACED", StringUtils::replaceAll("replaceitsomestringreplaceit", "replaceit", "REPLACED"));
}

/** 
 * Tests trim()
 * 
 */
TEST_F(StringUtilTest, trimTest) {
    EXPECT_EQ("somestring", StringUtils::trim("rrrtrimitttsomestringrrrtrimittttttt", "trimit"));
}

/** 
 * tests toString(), join()
 */
TEST_F(StringUtilTest, toStringTest) {
    EXPECT_EQ("12.34", StringUtils::toString(12.34));   //trivial implementation

    EXPECT_EQ("012.34", StringUtils::toString(12.34, strlen("12.34") +1, '0')); 
    EXPECT_EQ("12.34", StringUtils::toString(12.34, strlen("12.34") -1, '0'));

    // join()
    std::string str[] = {"one", "two", "three", "^_^"}; // "^_^" is used to indicate last token 
    std::vector<std::string> tokens;
    std::string delim("delim"), expected;

    for (int i = 0;  ; i++) {
        tokens.push_back(str[i]);        
        expected += str[i];
        if ("^_^" == str[i]) 
            break;  
        expected +=  delim;
    }

    EXPECT_EQ(expected, StringUtils::join(tokens, delim));

}

