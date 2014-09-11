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
#include <stdio.h>

#include "tgt/gpucapabilities.h"
#include "tgt/shadermanager.h"
#include "tgt/glcontextmanager.h"
#include "tgt/qt/qtthreadedcanvas.h"
#include "tgt/logmanager.h"
#include "tgt/qt/qtapplication.h"

#include "core/tools/simplejobprocessor.h"
#include "core/tools/opengljobprocessor.h"
#include "core/tools/quadrenderer.h"

#ifdef Q_WS_X11
#include <X11/Xlib.h>
#endif

QApplication *app;
/// Flag, whether CampVisApplication was correctly initialized
bool _initialized;
/// A local OpenGL context used for initialization
tgt::GLCanvas* _localContext = nullptr;
static const std::string loggerCat_;


void init() {
    // Make Xlib and GLX thread safe under X11
    QApplication::setAttribute(Qt::AA_X11InitThreads);

    sigslot::signal_manager::init();
    sigslot::signal_manager::getRef().setSignalHandlingMode(sigslot::signal_manager::FORCE_DIRECT);
    sigslot::signal_manager::getRef().start();

    tgt::GlContextManager::init();

    campvis::OpenGLJobProcessor::init();
    campvis::OpenGLJobProcessor::getRef().iKnowWhatImDoingSetThisThreadOpenGlThread();
    campvis::SimpleJobProcessor::init();

    tgtAssert(_initialized == false, "Tried to initialize CampVisApplication twice.");
    
    // Init TGT
    tgt::InitFeature::Features featureset = tgt::InitFeature::ALL;
    tgt::init(featureset);
    LogMgr.getConsoleLog()->addCat("", true, tgt::Info);

    // create a local OpenGL context and init GL
    _localContext = new tgt::QtThreadedCanvas("", tgt::ivec2(16, 16));
    
    tgt::GLContextScopedLock lock(_localContext);
    tgt::GlContextManager::getRef().registerContextAndInitGlew(_localContext);
    GLJobProc.iKnowWhatImDoingSetThisThreadOpenGlThread();
    GLJobProc.registerContext(_localContext);

    tgt::initGL(featureset);
    ShdrMgr.setDefaultGlslVersion("330");

    campvis::QuadRenderer::init();

#ifdef CAMPVIS_SOURCE_DIR
    {
        std::string sourcePath = CAMPVIS_SOURCE_DIR;
        ShdrMgr.addPath(sourcePath);
        ShdrMgr.addPath(sourcePath + "/core/glsl");
    }
#endif

    // ensure matching OpenGL specs
    LINFO("Using Graphics Hardware " << GpuCaps.getVendorAsString() << " " << GpuCaps.getGlRendererString() << " on " << GpuCaps.getOSVersionString());
    LINFO("Supported OpenGL " << GpuCaps.getGlVersion() << ", GLSL " << GpuCaps.getShaderVersion());
    if (GpuCaps.getGlVersion() < tgt::GpuCapabilities::GlVersion::TGT_GL_VERSION_3_3) {
        LERROR("Your system does not support OpenGL 3.3, which is mandatory. CAMPVis will probably not work as intended.");
    }
    if (GpuCaps.getShaderVersion() < tgt::GpuCapabilities::GlVersion::SHADER_VERSION_330) {
        LERROR("Your system does not support GLSL Shader Version 3.30, which is mandatory. CAMPVis will probably not work as intended.");
    }
    
    _initialized = true;
}

void deinit() {
    tgtAssert(_initialized, "Tried to deinitialize uninitialized CampVisApplication.");

    {
        // Deinit everything OpenGL related using the local context.
        tgt::GLContextScopedLock lock(_localContext);

        campvis::QuadRenderer::deinit();

        tgt::deinitGL();
    }

    campvis::SimpleJobProcessor::deinit();
    campvis::OpenGLJobProcessor::deinit();


    tgt::GlContextManager::deinit();
    tgt::deinit();

    sigslot::signal_manager::getRef().stop();
    sigslot::signal_manager::deinit();

    _initialized = false;
}

GTEST_API_ int main(int argc, char **argv) {
    printf("Running main() from main.cpp\n");

#ifdef Q_WS_X11
    XInitThreads();
#endif


    app = new QApplication(argc, argv);
    //testing::InitGoogleTest(&argc, argv);
    int _argc = 2;
    char *options[] = {"THIS DOESN'T HAVE ANY EFFECT", "--gtest_output=xml:visregtests/result.xml"};
    testing::InitGoogleTest(&_argc, options);

    int ret;

    init();
    {
        tgt::GLContextScopedLock lock(_localContext);
        ret= RUN_ALL_TESTS();

    }
    deinit();

    delete app;
    printf("main() returned with %d\n", ret);
    return 0;
}
