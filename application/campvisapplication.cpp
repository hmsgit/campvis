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

#include "campvisapplication.h"

#include "tgt/assert.h"
#include "tgt/exception.h"
#include "tgt/glcanvas.h"
#include "tgt/glcontextmanager.h"
#include "tgt/gpucapabilities.h"
#include "tgt/opengljobprocessor.h"
#include "tgt/shadermanager.h"
#include "tgt/texturereadertga.h"
#include "tgt/qt/qtapplication.h"
#include "tgt/qt/qtthreadedcanvas.h"

#include "application/campvispainter.h"
#include "application/gui/properties/propertywidgetfactory.h"
#include "application/gui/mainwindow.h"
#include "application/gui/mdi/mdidockablewindow.h"

#include "core/tools/simplejobprocessor.h"
#include "core/tools/stringutils.h"
#include "core/tools/quadrenderer.h"
#include "core/pipeline/abstractpipeline.h"
#include "core/datastructures/imagerepresentationconverter.h"
#include "core/pipeline/visualizationprocessor.h"

#include "modules/pipelinefactory.h"
#include "qtjobprocessor.h"

#ifdef CAMPVIS_HAS_SCRIPTING
#include "scripting/gen_pipelineregistration.h"
#endif

namespace campvis {

    const std::string CampVisApplication::loggerCat_ = "CAMPVis.application.CampVisApplication";

    CampVisApplication::CampVisApplication(int& argc, char** argv) 
        : QApplication(argc, argv)
        , _localContext(0)
        , _mainWindow(0)
        , _errorTexture(nullptr)
        , _luaVmState(nullptr)
        , _initialized(false)
        , _argc(argc)
        , _argv(argv)
    {
        // Make Xlib and GLX thread safe under X11
        QApplication::setAttribute(Qt::AA_X11InitThreads);

        sigslot::signal_manager::init();
        sigslot::signal_manager::getRef().start();
        tgt::GlContextManager::init();
        OpenGLJobProcessor::init();
        SimpleJobProcessor::init();
        QtJobProcessor::init();
    }

    CampVisApplication::~CampVisApplication() {
        tgtAssert(_initialized == false, "Destructing initialized CampVisApplication, deinitialize first!");

        // delete everything in the right order:
        for (std::vector<PipelineRecord>::iterator it = _pipelines.begin(); it != _pipelines.end(); ++it) {
            delete it->_painter;
            delete it->_pipeline;
        }
        for (std::vector<DataContainer*>::iterator it = _dataContainers.begin(); it != _dataContainers.end(); ++it) {
            delete *it;
        }

        sigslot::signal_manager::getRef().stop();
        sigslot::signal_manager::deinit();
    }

    void CampVisApplication::init() {
        tgtAssert(_initialized == false, "Tried to initialize CampVisApplication twice.");

        // Init TGT
        tgt::InitFeature::Features featureset = tgt::InitFeature::ALL;
        tgt::init(featureset);
        LogMgr.getConsoleLog()->addCat("", true);

        _mainWindow = new MainWindow(this);

        // create a local OpenGL context and init GL
        _localContext = new QtThreadedCanvas("", tgt::ivec2(16, 16));
        tgt::GlContextManager::getRef().registerContextAndInitGlew(_localContext, "Local Context");

        tgt::initGL(featureset);
        ShdrMgr.setDefaultGlslVersion("330");
        LGL_ERROR;

        QuadRenderer::init();
        
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
        }

        _mainWindow->init();

        // ensure matching OpenGL specs
        LINFO("Using Graphics Hardware " << GpuCaps.getVendorAsString() << " " << GpuCaps.getGlRendererString() << " on " << GpuCaps.getOSVersionString());
        LINFO("Supported OpenGL " << GpuCaps.getGlVersion() << ", GLSL " << GpuCaps.getShaderVersion());
        if (GpuCaps.getGlVersion() < tgt::GpuCapabilities::GlVersion::TGT_GL_VERSION_3_3) {
            LERROR("Your system does not support OpenGL 3.3, which is mandatory. CAMPVis will probably not work as intended.");
        }
        if (GpuCaps.getShaderVersion() < tgt::GpuCapabilities::GlVersion::SHADER_VERSION_330) {
            LERROR("Your system does not support GLSL Shader Version 3.30, which is mandatory. CAMPVis will probably not work as intended.");
        }


        // load textureData from file
        tgt::TextureReaderTga trt;
        _errorTexture = trt.loadTexture(ShdrMgr.completePath("application/data/no_input.tga"), tgt::Texture::LINEAR);


#ifdef CAMPVIS_HAS_SCRIPTING
        // create and store Lua VM for this very pipeline
        _luaVmState = new LuaVmState();
        _luaVmState->redirectLuaPrint();

        // Let Lua know where CAMPVis modules are located
        if (! _luaVmState->execString("package.cpath = '" CAMPVIS_LUA_MODS_PATH "'"))
            LERROR("Error setting up Lua VM.");

        // Load CAMPVis' core Lua module to have SWIG glue for AutoEvaluationPipeline available
        if (! _luaVmState->execString("require(\"campvis\")"))
            LERROR("Error setting up Lua VM.");
        if (! _luaVmState->execString("require(\"tgt\")"))
            LERROR("Error setting up Lua VM.");

        if (! _luaVmState->execString("pipelines = {}"))
            LERROR("Error setting up Lua VM.");

        if (! _luaVmState->execString("inspect = require 'inspect'"))
            LERROR("Error setting up Lua VM.");
#endif
        GLCtxtMgr.releaseContext(_localContext, false);

        // parse argument list and create pipelines
        QStringList pipelinesToAdd = this->arguments();
        for (int i = 1; i < pipelinesToAdd.size(); ++i) {
            DataContainer* dc = createAndAddDataContainer("DataContainer #" + StringUtils::toString(_dataContainers.size() + 1));
            AbstractPipeline* p = PipelineFactory::getRef().createPipeline(pipelinesToAdd[i].toStdString(), dc);
            if (p != 0)
                addPipeline(pipelinesToAdd[i].toStdString(), p);
        }

        GLJobProc.setContext(_localContext);
        GLJobProc.start();

        _initialized = true;
    }

    void CampVisApplication::deinit() {
        tgtAssert(_initialized, "Tried to deinitialize uninitialized CampVisApplication.");

        // Stop all pipeline threads.
        for (std::vector<PipelineRecord>::iterator it = _pipelines.begin(); it != _pipelines.end(); ++it) {
            it->_pipeline->stop();
        }

        {
            // Deinit everything OpenGL related using the local context.
            tgt::GLContextScopedLock lock(_localContext);

            delete _errorTexture;

            // Deinit pipeline and painter first
            for (std::vector<PipelineRecord>::iterator it = _pipelines.begin(); it != _pipelines.end(); ++it) {
                it->_pipeline->deinit();
                it->_painter->deinit();
            }

            _mainWindow->deinit();
            QuadRenderer::deinit();

            // deinit OpenGL and tgt
            tgt::deinitGL();
        }

        // MainWindow dtor needs a valid CampVisApplication, so we need to call it here instead of during destruction.
        delete _mainWindow;

        GLJobProc.stop();
        OpenGLJobProcessor::deinit();
        SimpleJobProcessor::deinit();

        tgt::GlContextManager::deinit();
        tgt::deinit();

        PropertyWidgetFactory::deinit();
        ImageRepresentationConverter::deinit();
        PipelineFactory::deinit();

        _initialized = false;
    }

    int CampVisApplication::run() {
        tgtAssert(_initialized, "Tried to run uninitialized CampVisApplication.");

        // disconnect OpenGL context from this thread so that the other threads can acquire an OpenGL context.
        //tgt::GlContextManager::getRef().releaseCurrentContext();

        _mainWindow->show();

        // Start QApplication
        int toReturn = QApplication::exec();

        return toReturn;
    }

    void CampVisApplication::addPipeline(const std::string& name, AbstractPipeline* pipeline) {
        tgtAssert(pipeline != 0, "Pipeline must not be 0.");

        // create canvas and painter for the pipeline and connect all together
        tgt::QtThreadedCanvas* canvas = new tgt::QtThreadedCanvas("CAMPVis", tgt::ivec2(512, 512));
        canvas->init();

        CampVisPainter* painter = new CampVisPainter(canvas, pipeline);
        canvas->setPainter(painter, false);
        pipeline->setCanvas(canvas);
        painter->setErrorTexture(_errorTexture);

        PipelineRecord pr = { pipeline, painter };
        _pipelines.push_back(pr);

        _pipelineWindows[pipeline] = _mainWindow->addVisualizationPipelineWidget(name, canvas);

        // initialize context (GLEW) and pipeline in OpenGL thread)
        initGlContextAndPipeline(canvas, pipeline);

#ifdef CAMPVIS_HAS_SCRIPTING
        if (! _luaVmState->injectObjectPointerToTable(pipeline, "campvis::AutoEvaluationPipeline *", "pipelines", static_cast<int>(_pipelines.size())))
        //if (! _luaVmState->injectObjectPointerToTableField(pipeline, "campvis::AutoEvaluationPipeline *", "pipelines", name))
            LERROR("Could not inject the pipeline into the Lua VM.");

        _luaVmState->execString("inspect(pipelines)");
#endif

        GLCtxtMgr.releaseContext(canvas, false);
        s_PipelinesChanged.emitSignal();
        pipeline->start();
    }

    void CampVisApplication::initGlContextAndPipeline(tgt::GLCanvas* canvas, AbstractPipeline* pipeline) {
        tgt::GlContextManager::getRef().registerContextAndInitGlew(canvas, pipeline->getName());

        pipeline->init();
        LGL_ERROR;
        canvas->getPainter()->init();
        LGL_ERROR;

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
        s_DataContainersChanged.emitSignal();
        return dc;
    }

    void CampVisApplication::rebuildAllShadersFromFiles() {
        // rebuilding all shaders has to be done from OpenGL context, use the local one.
        GLJobProc.enqueueJob(makeJobOnHeap(this, &CampVisApplication::triggerShaderRebuild));
    }

    void CampVisApplication::triggerShaderRebuild() {
        if (! ShdrMgr.rebuildAllShadersFromFile()) {
            LERROR("Could not rebuild all shaders from file.");
            return;
        }
        else {
            LINFO("Rebuilding shaders from file successful.");
        }

        for (std::vector<PipelineRecord>::iterator it = _pipelines.begin(); it != _pipelines.end(); ++it) {
            for (std::vector<AbstractProcessor*>::const_iterator pit = it->_pipeline->getProcessors().begin(); pit != it->_pipeline->getProcessors().end(); ++pit) {
                if (VisualizationProcessor* tester = dynamic_cast<VisualizationProcessor*>(*pit)) {
                	tester->invalidate(AbstractProcessor::INVALID_RESULT);
                }
            }
        }
    }

#ifdef CAMPVIS_HAS_SCRIPTING
    LuaVmState* CampVisApplication::getLuaVmState() {
        return _luaVmState;
    }

#endif

}
