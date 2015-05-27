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
#include <QApplication>

#include "sigslot/sigslot.h"

#include "cgt/glcontextmanager.h"
#include "cgt/gpucapabilities.h"
#include "cgt/init.h"
#include "cgt/logmanager.h"
#include "cgt/opengljobprocessor.h"
#include "cgt/shadermanager.h"
#include "cgt/qt/qtthreadedcanvas.h"

#include "core/tools/simplejobprocessor.h"
#include "core/tools/quadrenderer.h"

QApplication *app;
/// Flag, whether CampVisApplication was correctly initialized
bool _initialized;
/// A local OpenGL context used for initialization
cgt::GLCanvas* _localContext = nullptr;
static const std::string loggerCat_;


void init() {
    // Make Xlib and GLX thread safe under X11
    QApplication::setAttribute(Qt::AA_X11InitThreads);

    sigslot::signal_manager::init();
    sigslot::signal_manager::getRef().setSignalHandlingMode(sigslot::signal_manager::FORCE_DIRECT);
    sigslot::signal_manager::getRef().start();

    cgt::GlContextManager::init();

    cgt::OpenGLJobProcessor::init();
    campvis::SimpleJobProcessor::init();

    cgtAssert(_initialized == false, "Tried to initialize CampVisApplication twice.");
    
    // Init CGT
    cgt::InitFeature::Features featureset = cgt::InitFeature::ALL;
    cgt::init(featureset);
    LogMgr.getConsoleLog()->addCat("", true, cgt::Info);

    // create a local OpenGL context and init GL
    cgt::QtThreadedCanvas* backgroundCanvas = new cgt::QtThreadedCanvas("", cgt::ivec2(16, 16));
    GLCtxtMgr.registerContextAndInitGlew(backgroundCanvas, "Background Context");
    GLCtxtMgr.releaseContext(backgroundCanvas, false);
    GLJobProc.setContext(backgroundCanvas);
    GLJobProc.start();
    
    _localContext = new cgt::QtThreadedCanvas("", cgt::ivec2(16, 16));
    cgt::GlContextManager::getRef().registerContextAndInitGlew(_localContext, "Local Context");

    cgt::initGL(featureset);
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
    if (GpuCaps.getGlVersion() < cgt::GpuCapabilities::GlVersion::CGT_GL_VERSION_3_3) {
        LERROR("Your system does not support OpenGL 3.3, which is mandatory. CAMPVis will probably not work as intended.");
    }
    if (GpuCaps.getShaderVersion() < cgt::GpuCapabilities::GlVersion::SHADER_VERSION_330) {
        LERROR("Your system does not support GLSL Shader Version 3.30, which is mandatory. CAMPVis will probably not work as intended.");
    }

    GLCtxtMgr.releaseContext(_localContext, false);

    _initialized = true;
}

void deinit() {
    cgtAssert(_initialized, "Tried to deinitialize uninitialized CampVisApplication.");

    {
        // Deinit everything OpenGL related using the local context.
        cgt::GLContextScopedLock lock(_localContext);

        campvis::QuadRenderer::deinit();

        campvis::SimpleJobProcessor::deinit();
        GLJobProc.stop();
        cgt::OpenGLJobProcessor::deinit();

        cgt::deinitGL();
    }


    cgt::GlContextManager::deinit();
    cgt::deinit();

    sigslot::signal_manager::getRef().stop();
    sigslot::signal_manager::deinit();

    _initialized = false;
}

GTEST_API_ int main(int argc, char **argv) {
    printf("Running main() from main.cpp\n");

    app = new QApplication(argc, argv);

    // Make Xlib and GLX thread safe under X11
    QApplication::setAttribute(Qt::AA_X11InitThreads);

    testing::InitGoogleTest(&argc, argv);
    int ret;

    init();
    {
        cgt::GLContextScopedLock lock(_localContext);
        ret= RUN_ALL_TESTS();

    }
    deinit();

    delete app;
    printf("main() returned with %d\n", ret);
    return 0;
}
