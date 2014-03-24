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

//#include "tgt/assert.h"
//#include "tgt/exception.h"
//#include "tgt/glcanvas.h"
#include "tgt/gpucapabilities.h"
#include "tgt/shadermanager.h"
#include "tgt/qt/qtapplication.h"
//#include "tgt/qt/qtthreadedcanvas.h"
#include "tgt/qt/qtcontextmanager.h"
#include "tgt/logmanager.h"
//#include "tbb/compat/thread"

#include "core/tools/simplejobprocessor.h"
#include "core/tools/opengljobprocessor.h"
#include "core/tools/quadrenderer.h"

//using namespace campvis;

/// Not sure, whether or not we need it!!
QApplication *app;

/// Flag, whether CampVisApplication was correctly initialized
bool _initialized;

/// A local OpenGL context used for initialization
/**?
 * how this variable is related with tgt macros!! 
 */
tgt::GLCanvas* _localContext = nullptr;

static const std::string loggerCat_;


void init() {
    // Make Xlib and GLX thread safe under X11
    QApplication::setAttribute(Qt::AA_X11InitThreads);

    tgt::QtContextManager::init();

    campvis::OpenGLJobProcessor::init();
    campvis::SimpleJobProcessor::init();

    // Init TGT
    tgt::InitFeature::Features featureset = tgt::InitFeature::ALL;
    tgt::init(featureset);
    LogMgr.getConsoleLog()->addCat("", true);

    // create a local OpenGL context and init GL
    _localContext = tgt::GlContextManager::getRef().createContext("AppContext", "", tgt::ivec2(16, 16));
    tgtAssert(nullptr != _localContext, "Could not create local OpenGL context");

    tgt::GLContextScopedLock lock(_localContext);

    tgt::initGL(featureset);
    ShdrMgr.setDefaultGlslVersion("330");


    campvis::QuadRenderer::init();

    // ensure matching OpenGL specs
    LINFO("Using Graphics Hardware " << GpuCaps.getVendorAsString() << " " << GpuCaps.getGlRendererString() << " on " << GpuCaps.getOSVersionString());
    LINFO("Supported OpenGL " << GpuCaps.getGlVersion() << ", GLSL " << GpuCaps.getShaderVersion());
    if (GpuCaps.getGlVersion() < tgt::GpuCapabilities::GlVersion::TGT_GL_VERSION_3_3) {
        LERROR("Your system does not support OpenGL 3.3, which is mandatory. CAMPVis will probably not work as intended.");
    }
    if (GpuCaps.getShaderVersion() < tgt::GpuCapabilities::GlVersion::SHADER_VERSION_330) {
        LERROR("Your system does not support GLSL Shader Version 3.30, which is mandatory. CAMPVis will probably not work as intended.");
    }

    /**?
     * had to change macro in original file, else OpenGLJobProcessor is not known here
     * unless "using" statement is written on the top.
     */
    GLJobProc.start();
    GLJobProc.registerContext(_localContext);

    _initialized = true;
}

void deinit() {
    tgtAssert(_initialized, "Tried to deinitialize uninitialized CampVisApplication.");

    GLJobProc.stop();

    /**?
     * Grrrrrrrrrrrrrrrr :#
     * What with this BLOCK :-/ without it the program doesn't exit.
     * OpenGLJobProcessor::deinit() halts in Singleton::deinit()
     * WHY IS STACK SCOPE IMPORTANT HERE!!!
     */
    {
        // Deinit everything OpenGL related using the local context.
        tgt::GLContextScopedLock lock(_localContext);

        campvis::QuadRenderer::deinit();

        // deinit OpenGL and tgt
        tgt::deinitGL();

    }

    campvis::SimpleJobProcessor::deinit();
    campvis::OpenGLJobProcessor::deinit();


    tgt::QtContextManager::deinit();
    tgt::deinit();

    _initialized = false;
}

GTEST_API_ int main(int argc, char **argv) {
    printf("Running main() from main.cpp\n");
    app = new QApplication(argc, argv);
    testing::InitGoogleTest(&argc, argv);

    init();

    int ret= RUN_ALL_TESTS();

    deinit();

    //getchar();
    delete app;
    printf("main() returned with %d\n", ret);
    return 0;
}
