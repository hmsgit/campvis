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

#include "cgt/assert.h"
#include "cgt/exception.h"
#include "cgt/glcanvas.h"
#include "cgt/glcontextmanager.h"
#include "cgt/gpucapabilities.h"
#include "cgt/init.h"
#include "cgt/opengljobprocessor.h"
#include "cgt/shadermanager.h"
#include "cgt/texturereadertga.h"
#include "cgt/qt/qtthreadedcanvas.h"

#include "application/gui/properties/propertywidgetfactory.h"
#include "application/gui/mainwindow.h"
#include "application/gui/mdi/mdidockablewindow.h"

#include "core/init.h"
#include "core/tools/simplejobprocessor.h"
#include "core/tools/stringutils.h"
#include "core/tools/quadrenderer.h"
#include "core/pipeline/abstractpipeline.h"
#include "core/pipeline/abstractworkflow.h"
#include "core/pipeline/pipelinepainter.h"
#include "core/datastructures/imagerepresentationconverter.h"
#include "core/pipeline/visualizationprocessor.h"

#include "modules/pipelinefactory.h"
#include "qtjobprocessor.h"

#include <QApplication>

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

    }

    CampVisApplication::~CampVisApplication() {
        cgtAssert(_initialized == false, "Destructing initialized CampVisApplication, deinitialize first!");
    }

    void CampVisApplication::init() {
        cgtAssert(_initialized == false, "Tried to initialize CampVisApplication twice.");

        std::vector<std::string> searchPaths;
        if (_argc > 0) {
            // ugly hack
            std::string basePath(_argv[0]);
            searchPaths.push_back(cgt::FileSystem::parentDir(basePath));
            searchPaths.push_back(cgt::FileSystem::parentDir(cgt::FileSystem::parentDir(basePath)));
#ifdef CAMPVIS_SOURCE_DIR
            searchPaths.push_back(CAMPVIS_SOURCE_DIR);
#endif
        }

        _localContext = new QtThreadedCanvas("", cgt::ivec2(16, 16));
        campvis::init(_localContext, searchPaths);

        _mainWindow = new MainWindow(this);
        cgt::GLContextScopedLock lock(_localContext);
        {
            _mainWindow->init();

            // load textureData from file
            cgt::TextureReaderTga trt;
            _errorTexture = trt.loadTexture(ShdrMgr.completePath("application/data/no_input.tga"), cgt::Texture::LINEAR);

#ifdef CAMPVIS_HAS_SCRIPTING
            // create and store Lua VM for this very pipeline
            _luaVmState = new LuaVmState();
            _luaVmState->redirectLuaPrint();

            // Let Lua know where CAMPVis modules are located
            if (! _luaVmState->execString("package.cpath = '" CAMPVIS_LUA_MODS_PATH "'"))
                LERROR("Error setting up Lua VM.");
            if (! _luaVmState->execString("package.path = package.path .. ';" CAMPVIS_LUA_SCRIPTS_PATH "'"))
                LERROR("Error setting up Lua VM.");

            // Load CAMPVis' core Lua module to have SWIG glue for AutoEvaluationPipeline available
            if (! _luaVmState->execString("require(\"campvis\")"))
                LERROR("Error setting up Lua VM.");
            if (! _luaVmState->execString("require(\"cgt\")"))
                LERROR("Error setting up Lua VM.");

            if (! _luaVmState->execString("pipelines = {}"))
                LERROR("Error setting up Lua VM.");

            if (! _luaVmState->execString("inspect = require 'inspect'"))
                LERROR("Error setting up Lua VM.");
#endif
        }

        // parse argument list and create pipelines
        QStringList pipelinesToAdd = this->arguments();
        for (int i = 1; i < pipelinesToAdd.size(); ++i) {
            if (pipelinesToAdd[i] == "-w" && i+1 < pipelinesToAdd.size()) {
                // create workflow
                AbstractWorkflow* w = PipelineFactory::getRef().createWorkflow(pipelinesToAdd[i+1].toStdString());

                if (w != nullptr) {
                    // get DataContainers and Pipelines from workflow, take ownership and initialize them where necessary
                    _dataContainers.push_back(w->getDataContainer());
                    s_DataContainersChanged.emitSignal();

                    std::vector<AbstractPipeline*> pipelines = w->getPipelines();
                    for (auto it = pipelines.begin(); it != pipelines.end(); ++it) {
                        addPipeline((*it)->getName(), *it);
                    }

                    _workflows.push_back(w);
                    _mainWindow->setWorkflow(w);
                    w->init();
                }

                ++i;
            }
            else if (pipelinesToAdd[i] == "-k") {
                // set kiosk mode
                _mainWindow->enableKioskMode();
            }
            else {
                DataContainer* dc = createAndAddDataContainer("DataContainer #" + StringUtils::toString(_dataContainers.size() + 1));
                AbstractPipeline* p = PipelineFactory::getRef().createPipeline(pipelinesToAdd[i].toStdString(), dc);
                if (p != nullptr)
                    addPipeline(pipelinesToAdd[i].toStdString(), p);
            }
        }
        
        _initialized = true;
    }

    void CampVisApplication::deinit() {
        cgtAssert(_initialized, "Tried to deinitialize uninitialized CampVisApplication.");

        // Stop all pipeline threads.
        for (auto it = _pipelines.begin(); it != _pipelines.end(); ++it) {
            (*it)->stop();
        }

        for (auto it = _workflows.begin(); it != _workflows.end(); ++it)
            (*it)->deinit();

        {
            // Deinit everything OpenGL related using the local context.
            cgt::GLContextScopedLock lock(_localContext);

            delete _errorTexture;

            // Deinit pipeline and painter first
            for (auto it = _pipelines.begin(); it != _pipelines.end(); ++it) {
                (*it)->deinit();
            }

            _mainWindow->deinit();
        }

        // now delete everything in the right order:
        for (auto it = _pipelines.begin(); it != _pipelines.end(); ++it) {
            delete *it;
        }
        for (auto it = _dataContainers.begin(); it != _dataContainers.end(); ++it) {
            delete *it;
        }

        campvis::deinit();
        PropertyWidgetFactory::deinit();
        PipelineFactory::deinit();

        _initialized = false;
    }

    int CampVisApplication::run() {
        cgtAssert(_initialized, "Tried to run uninitialized CampVisApplication.");

        // disconnect OpenGL context from this thread so that the other threads can acquire an OpenGL context.
        //cgt::GlContextManager::getRef().releaseCurrentContext();

        _mainWindow->show();

        // Start QApplication
        int toReturn = QApplication::exec();

        return toReturn;
    }

    void CampVisApplication::addPipeline(const std::string& name, AbstractPipeline* pipeline) {
        cgtAssert(pipeline != 0, "Pipeline must not be 0.");

        // create canvas and painter for the pipeline and connect all together
        cgt::QtThreadedCanvas* canvas = new cgt::QtThreadedCanvas("CAMPVis", cgt::ivec2(512, 512));
        canvas->init();

        pipeline->setCanvas(canvas);
        pipeline->getPipelinePainter()->setErrorTexture(_errorTexture);

        _pipelines.push_back(pipeline);
        _pipelineWindows[pipeline] = _mainWindow->addVisualizationPipelineWidget(name, canvas);

        // initialize context (GLEW) and pipeline in OpenGL thread)
        initGlContextAndPipeline(canvas, pipeline);

#ifdef CAMPVIS_HAS_SCRIPTING
        if (! _luaVmState->injectObjectPointerToTable(pipeline, "campvis::AutoEvaluationPipeline *", "pipelines", static_cast<int>(_pipelines.size())))
        //if (! _luaVmState->injectObjectPointerToTableField(pipeline, "campvis::AutoEvaluationPipeline *", "pipelines", name))
            LERROR("Could not inject the pipeline into the Lua VM.");

        if (! _luaVmState->injectObjectPointerToTableField(pipeline, "campvis::AutoEvaluationPipeline *", "pipelines", pipeline->getName()))
            LERROR("Could not inject the pipeline into the Lua VM.");

        _luaVmState->execString("inspect(pipelines)");
#endif

        GLCtxtMgr.releaseContext(canvas, false);
        s_PipelinesChanged.emitSignal();
        pipeline->start();
    }

    void CampVisApplication::initGlContextAndPipeline(cgt::GLCanvas* canvas, AbstractPipeline* pipeline) {
        cgt::GlContextManager::getRef().registerContextAndInitGlew(canvas, pipeline->getName());

        pipeline->init();
        LGL_ERROR;

        // enable pipeline and invalidate all processors
        pipeline->setEnabled(true);
        for (std::vector<AbstractProcessor*>::const_iterator it = pipeline->getProcessors().begin(); it != pipeline->getProcessors().end(); ++it) {
            (*it)->invalidate(AbstractProcessor::INVALID_RESULT);
        }
    }

    void CampVisApplication::registerDockWidget(Qt::DockWidgetArea area, QDockWidget* dock) {
        cgtAssert(dock != 0, "Dock widget must not be 0.");

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

        for (auto it = _pipelines.begin(); it != _pipelines.end(); ++it) {
            for (auto pit = (*it)->getProcessors().cbegin(); pit != (*it)->getProcessors().cend(); ++pit) {
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

    void CampVisApplication::setPipelineVisibility(AbstractPipeline* pipeline, bool visibility) {
        auto it = _pipelineWindows.find(pipeline);
        if (it != _pipelineWindows.end()) {
            it->second->setVisible(visibility);
        }
    }

}
