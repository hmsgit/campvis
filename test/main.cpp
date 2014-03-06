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
