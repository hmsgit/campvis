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
#include "core/tools/stringutils.h"
#include "core/tools/quadrenderer.h"
#include "core/pipeline/abstractpipeline.h"
#include "modules/pipelinefactory.h"

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
        for (std::vector< std::pair<AbstractPipeline*, CampVisPainter*> >::iterator it = _visualizations.begin(); it != _visualizations.end(); ++it) {
            delete it->second;
        }
        for (std::vector<AbstractPipeline*>::iterator it = _pipelines.begin(); it != _pipelines.end(); ++it) {
            delete *it;
        }
        for (std::vector<DataContainer*>::iterator it = _dataContainers.begin(); it != _dataContainers.end(); ++it) {
            delete *it;
        }
    }

    void CampVisApplication::init() {
        tgtAssert(_initialized == false, "Tried to initialize CampVisApplication twice.");

        // parse argument list and create pipelines
        QStringList pipelinesToAdd = this->arguments();
        for (int i = 1; i < pipelinesToAdd.size(); ++i) {
            DataContainer* dc = createAndAddDataContainer("DataContainer #" + StringUtils::toString(_dataContainers.size() + 1));
            AbstractPipeline* p = PipelineFactory::getRef().createPipeline(pipelinesToAdd[i].toStdString(), dc);
            if (p != 0)
                addPipeline(pipelinesToAdd[i].toStdString(), p);
        }

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
        for (std::vector< std::pair<AbstractPipeline*, CampVisPainter*> >::iterator it = _visualizations.begin(); it != _visualizations.end(); ++it) {
            it->second->init();
        }

        GLJobProc.start();
        _initialized = true;
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
            for (std::vector< std::pair<AbstractPipeline*, CampVisPainter*> >::iterator it = _visualizations.begin(); it != _visualizations.end(); ++it) {
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

    void CampVisApplication::addPipeline(const std::string& name, AbstractPipeline* pipeline) {
        tgtAssert(pipeline != 0, "Pipeline must not be 0.");

        // if CAMPVis is already fully initialized, we need to temporarily shut down its
        // OpenGL job processor, since we need to create a new context.
        if (_initialized) {
            GLJobProc.pause();
            {
                tgt::QtThreadedCanvas* canvas = CtxtMgr.createContext(name, "CAMPVis", tgt::ivec2(512, 512));
                tgt::GLContextScopedLock lock(canvas->getContext());
                addPipelineImpl(canvas, name, pipeline);
            }
            GLJobProc.resume();
        }
        else {
            tgt::QtThreadedCanvas* canvas = CtxtMgr.createContext(name, "CAMPVis", tgt::ivec2(512, 512));
            addPipelineImpl(canvas, name, pipeline);
        }

        s_PipelinesChanged();
    }

    void CampVisApplication::addPipelineImpl(tgt::QtThreadedCanvas* canvas, const std::string& name, AbstractPipeline* pipeline) {
        // create canvas and painter for the pipeline and connect all together
        
        GLJobProc.registerContext(canvas);
        canvas->init();

        CampVisPainter* painter = new CampVisPainter(canvas, pipeline);
        canvas->setPainter(painter, false);
        pipeline->setCanvas(canvas);

        _visualizations.push_back(std::make_pair(pipeline, painter));
        _pipelines.push_back(pipeline);

        if (_initialized) {
            LGL_ERROR;
            pipeline->init();
            LGL_ERROR;
            painter->init();
            LGL_ERROR;
        }

        CtxtMgr.releaseCurrentContext();
        _mainWindow->addVisualizationPipelineWidget(name, canvas);

        // enable pipeline and invalidate all processors
        pipeline->setEnabled(true);
        for (std::vector<AbstractProcessor*>::const_iterator it = pipeline->getProcessors().begin(); it != pipeline->getProcessors().end(); ++it) {
            (*it)->invalidate(AbstractProcessor::INVALID_RESULT);
        }
    }

    void CampVisApplication::registerDockWidget(Qt::DockWidgetArea area, QDockWidget* dock) {
        tgtAssert(dock != 0, "Dock widget must not be 0.");

        _mainWindow->addDockWidget(area, dock);
    }

    DataContainer* CampVisApplication::createAndAddDataContainer(const std::string& name) {
        DataContainer* dc = new DataContainer(name);
        _dataContainers.push_back(dc);
        s_DataContainersChanged();
        return dc;
    }


}
