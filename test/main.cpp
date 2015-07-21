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
#include <stdio.h>
#include <QApplication>

#include "sigslot/sigslot.h"

#include "cgt/glcontextmanager.h"
#include "cgt/gpucapabilities.h"
#include "cgt/init.h"
#include "cgt/logmanager.h"
#include "cgt/opengljobprocessor.h"
#include "cgt/shadermanager.h"
#include "cgt/qt/qtthreadedcanvas.h"

#include "core/init.h"
#include "core/tools/quadrenderer.h"

QApplication *app;
/// Flag, whether CampVisApplication was correctly initialized
bool _initialized;
/// A local OpenGL context used for initialization
cgt::GLCanvas* _localContext = nullptr;
static const std::string loggerCat_;


void init() {
    std::vector<std::string> searchPaths;
#ifdef CAMPVIS_SOURCE_DIR
        searchPaths.push_back(CAMPVIS_SOURCE_DIR);
#endif

    cgt::QtThreadedCanvas* backgroundCanvas = new cgt::QtThreadedCanvas("", cgt::ivec2(16, 16));
    campvis::init(backgroundCanvas, searchPaths);
    
    _localContext = new cgt::QtThreadedCanvas("", cgt::ivec2(16, 16));
    cgt::GlContextManager::getRef().registerContextAndInitGlew(_localContext, "Local Context");
    GLCtxtMgr.releaseContext(_localContext, false);

    _initialized = true;
}

void deinit() {
    cgtAssert(_initialized, "Tried to deinitialize uninitialized CampVisApplication.");
    campvis::deinit();

    _initialized = false;
}

GTEST_API_ int main(int argc, char **argv) {
    printf("Running main() from main.cpp\n");

    // Make Xlib and GLX thread safe under X11
    QCoreApplication::setAttribute(Qt::AA_X11InitThreads);
    app = new QApplication(argc, argv);

    testing::InitGoogleTest(&argc, argv);
    init();
    int ret;
    
    std::thread testThread([&] () {
        cgt::GLContextScopedLock lock(_localContext);
        ret= RUN_ALL_TESTS();
        app->exit();
    });

    app->exec();
    deinit();

    delete app;
    printf("main() returned with %d\n", ret);
    return 0;
}
