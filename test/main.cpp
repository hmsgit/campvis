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
#include <stdio.h>

#include "core/tools/simplejobprocessor.h"

void init() {
    campvis::SimpleJobProcessor::init();
}

void deinit() {
    campvis::SimpleJobProcessor::deinit();
}

GTEST_API_ int main(int argc, char **argv) {
    printf("Running main() from main.cpp\n");
    testing::InitGoogleTest(&argc, argv);

    init();

    int ret= RUN_ALL_TESTS();
    printf("main() returned with %d\n", ret);

    deinit();

    //getchar();
    return 0;
}
