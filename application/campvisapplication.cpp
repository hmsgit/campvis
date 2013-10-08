// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#include "campvisapplication.h"

#include "tgt/assert.h"
#include "tgt/exception.h"
#include "tgt/glcontext.h"
#include "tgt/gpucapabilities.h"
#include "tgt/shadermanager.h"
#include "tgt/qt/qtapplication.h"
#include "tgt/qt/qtthreadedcanvas.h"
#include "tgt/qt/qtcontextmanager.h"
#include "tbb/compat/thread"

#ifdef HAS_KISSCL
#include "kisscl/clruntime.h"
#endif

#include "application/campvispainter.h"
#include "application/gui/mainwindow.h"
#include "core/tools/opengljobprocessor.h"
#include "core/tools/simplejobprocessor.h"
#include "core/tools/quadrenderer.h"
#include "core/pipeline/abstractpipeline.h"
#include "core/pipeline/autoevaluationpipeline.h"
#include "core/pipeline/pipelinefactory.h"

namespace campvis {

    const std::string CampVisApplication::loggerCat_ = "CAMPVis.application.CampVisApplication";

    CampVisApplication::CampVisApplication(int& argc, char** argv, bool useOpenCL) 
        : QApplication(argc, argv)
        , _localContext(0)
        , _mainWindow(0)
        , _initialized(false)
        , _useOpenCL(useOpenCL)
        , _argc(argc)
        , _argv(argv)
    {
        // Make Xlib and GLX thread safe under X11
        QApplication::setAttribute(Qt::AA_X11InitThreads);

        _mainWindow = new MainWindow(this);
        tgt::QtContextManager::init();

        OpenGLJobProcessor::init();
        SimpleJobProcessor::init();
    }

    CampVisApplication::~CampVisApplication() {
        tgtAssert(_initialized == false, "Destructing initialized CampVisApplication, deinitialize first!");

        // delete everything in the right order:
        for (std::vector< std::pair<AutoEvaluationPipeline*, CampVisPainter*> >::iterator it = _visualizations.begin(); it != _visualizations.end(); ++it) {
            delete it->second;
        }
        for (std::vector<AbstractPipeline*>::iterator it = _pipelines.begin(); it != _pipelines.end(); ++it) {
            delete *it;
        }
    }

    void CampVisApplication::init() {
        tgtAssert(_initialized == false, "Tried to initialize CampVisApplication twice.");

        // Init TGT
        tgt::InitFeature::Features featureset = tgt::InitFeature::ALL;
        tgt::init(featureset);
        LogMgr.getConsoleLog()->addCat("", true);

        // create a local OpenGL context and init GL
        _localContext = CtxtMgr.createContext("AppContext", "", tgt::ivec2(16, 16));
        tgtAssert(_localContext != 0, "Could not create local OpenGL context");

        tgt::GLContextScopedLock lock(_localContext->getContext());

        tgt::initGL(featureset);
        ShdrMgr.setGlobalHeader("#version 330\n");
        LGL_ERROR;

        // ensure matching OpenGL specs
        if (GpuCaps.getGlVersion() < tgt::GpuCapabilities::GlVersion::TGT_GL_VERSION_3_3) {
            LERROR("Your system does not support OpenGL 3.3, which is mandatory. CAMPVis will probably not work as intended.");
        }
        if (GpuCaps.getShaderVersion() < tgt::GpuCapabilities::GlVersion::SHADER_VERSION_330) {
            LERROR("Your system does not support GLSL Shader Version 3.30, which is mandatory. CAMPVis will probably not work as intended.");
        }
        if (!GpuCaps.isNpotSupported() && !GpuCaps.areTextureRectanglesSupported()) {
            LERROR("Neither non-power-of-two textures nor texture rectangles seem to be supported. CAMPVis will probably not work as intended.");
        }

        QuadRenderer::init();

#ifdef HAS_KISSCL
        if (_useOpenCL) {
            kisscl::CLRuntime::init();
        }
#endif

        if (_argc > 0) {
            // ugly hack
            std::string basePath(_argv[0]);
            basePath = tgt::FileSystem::parentDir(basePath);
            ShdrMgr.addPath(basePath);
            ShdrMgr.addPath(basePath + "/core/glsl");

            basePath = tgt::FileSystem::parentDir(tgt::FileSystem::parentDir(basePath));
            ShdrMgr.addPath(basePath);
            ShdrMgr.addPath(basePath + "/core/glsl");

#ifdef CAMPVIS_SOURCE_DIR
            {
                std::string sourcePath = CAMPVIS_SOURCE_DIR;
                ShdrMgr.addPath(sourcePath);
                ShdrMgr.addPath(sourcePath + "/core/glsl");
            }
#endif

#ifdef HAS_KISSCL
            if (_useOpenCL) {
                CLRtm.addPath(basePath);
                CLRtm.addPath(basePath + "/core/cl");
            }

#ifdef CAMPVIS_SOURCE_DIR
            {
                std::string sourcePath = CAMPVIS_SOURCE_DIR;
                CLRtm.addPath(sourcePath);
                CLRtm.addPath(sourcePath + "/core/glsl");
            }
#endif

#endif
        }

        _mainWindow->init();

        // init pipeline first
        for (std::vector<AbstractPipeline*>::iterator it = _pipelines.begin(); it != _pipelines.end(); ++it) {
            (*it)->init();
        }

        // Now init painters:
        for (std::vector< std::pair<AutoEvaluationPipeline*, CampVisPainter*> >::iterator it = _visualizations.begin(); it != _visualizations.end(); ++it) {
            it->second->init();
        }

        GLJobProc.start();
        _initialized = true;

        LINFO(PipelineFactory::getRef().toString());
    }

    void CampVisApplication::deinit() {
        tgtAssert(_initialized, "Tried to deinitialize uninitialized CampVisApplication.");

        GLJobProc.stop();

        {
            // Deinit everything OpenGL related using the local context.
            tgt::GLContextScopedLock lock(_localContext->getContext());

            // Deinit pipeline first
            for (std::vector<AbstractPipeline*>::iterator it = _pipelines.begin(); it != _pipelines.end(); ++it) {
                (*it)->deinit();
            }

            // Now deinit painters:
            for (std::vector< std::pair<AutoEvaluationPipeline*, CampVisPainter*> >::iterator it = _visualizations.begin(); it != _visualizations.end(); ++it) {
                it->second->deinit();
            }

            _mainWindow->deinit();

            QuadRenderer::deinit();

#ifdef HAS_KISSCL
            if (_useOpenCL) {
                kisscl::CLRuntime::deinit();
            }
#endif


            // deinit OpenGL and tgt
            tgt::deinitGL();
        }

        SimpleJobProcessor::deinit();
        OpenGLJobProcessor::deinit();

        tgt::QtContextManager::deinit();
        tgt::deinit();

        // MainWindow dtor needs a valid CampVisApplication, so we need to call it here instead of during destruction.
        delete _mainWindow;

        _initialized = false;
    }

    int CampVisApplication::run() {
        tgtAssert(_initialized, "Tried to run uninitialized CampVisApplication.");

        // disconnect OpenGL context from this thread so that the other threads can acquire an OpenGL context.
        CtxtMgr.releaseCurrentContext();

        _mainWindow->show();

        // Start QApplication
        int toReturn = QApplication::exec();

        return toReturn;
    }

    void CampVisApplication::addPipeline(AbstractPipeline* pipeline) {
        tgtAssert(_initialized == false, "Adding pipelines after initialization is currently not supported.");
        tgtAssert(pipeline != 0, "Pipeline must not be 0.");
        _pipelines.push_back(pipeline);

        s_PipelinesChanged();
    }

    void CampVisApplication::addVisualizationPipeline(const std::string& name, AutoEvaluationPipeline* vp) {
        tgtAssert(_initialized == false, "Adding pipelines after initialization is currently not supported.");
        tgtAssert(vp != 0, "Pipeline must not be 0.");

        // create canvas and painter for the VisPipeline and connect all together
        tgt::QtThreadedCanvas* canvas = CtxtMgr.createContext(name, "CAMPVis", tgt::ivec2(512, 512));
        GLJobProc.registerContext(canvas);
        _mainWindow->addVisualizationPipelineWidget(name, canvas);
        canvas->init();

        CampVisPainter* painter = new CampVisPainter(canvas, vp);
        canvas->setPainter(painter, false);

        _visualizations.push_back(std::make_pair(vp, painter));

        vp->setCanvas(canvas);
        addPipeline(vp);

        CtxtMgr.releaseCurrentContext();
    }

    void CampVisApplication::registerDockWidget(Qt::DockWidgetArea area, QDockWidget* dock) {
        tgtAssert(dock != 0, "Dock widget must not be 0.");

        _mainWindow->addDockWidget(area, dock);
    }

    DataContainer* CampVisApplication::getDataContainer() {
        return &_dataContainer;
    }

    const DataContainer* CampVisApplication::getDataContainer() const {
        return &_dataContainer;
    }

}
