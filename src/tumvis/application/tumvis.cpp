#include "tgt/camera.h"
#include "tgt/exception.h"
#include "tgt/shadermanager.h"
#include "tgt/qt/qtapplication.h"
#include "tgt/qt/qtcanvas.h"
#include "tgt/qt/qtcontextmanager.h"

#include "tbb/include/tbb/task_scheduler_init.h"
#include "tbb/include/tbb/compat/thread"

#include "tumvispainter.h"
#include "core/pipeline/pipelineevaluator.h"
#include "modules/pipelines/slicevis.h"

using namespace TUMVis;

SliceVis* sliceVis = 0;
TumVisPainter* painter = 0;
PipelineEvaluator* pe;

/**
 * TUMVis main function, application entry point
 *
 * \param   argc    number of passed arguments
 * \param   argv    vector of arguments
 * \return  0 if program exited successfully
 **/
int main(int argc, char** argv) {  
    tgt::QtApplication* app = new tgt::QtApplication(argc, argv);
    tgt::QtContextManager::init();
    tgt::QtThreadedCanvas* renderCanvas = CtxtMgr.createContext("render", "TUMVis");
    tgt::QtThreadedCanvas* sliceVisCanvas = CtxtMgr.createContext("sliceVis", "SliceVis");
    
    tbb::task_scheduler_init init;
    renderCanvas->getContext()->acquire();
    app->addCanvas(renderCanvas);
    app->init();
    LogMgr.getConsoleLog()->addCat("", true);
    LGL_ERROR;

    if (argc > 0) {
        // ugly hack
        std::string programPath(argv[0]);
        programPath = tgt::FileSystem::parentDir(tgt::FileSystem::parentDir(tgt::FileSystem::parentDir(programPath)));
        ShdrMgr.addPath(programPath);
        ShdrMgr.addPath(programPath + "/core/glsl");
    }
    LGL_ERROR;

    tgt::Camera camera;
    renderCanvas->setCamera(&camera); 


    try {
        sliceVis = new SliceVis(sliceVisCanvas);
        painter = new TumVisPainter(renderCanvas, sliceVis);
        LGL_ERROR;
        renderCanvas->setPainter(painter);
        LGL_ERROR;
        sliceVis->init();
        LGL_ERROR;
    }
    catch (tgt::Exception& e) {
        LERRORC("main.cpp", "Encountered tgt::Exception: " << e.what());
    }
    catch (std::exception& e) {
        LERRORC("main.cpp", "Encountered std::exception: " << e.what());
    }

    // disconnect OpenGL context from this thread so that the other threads can acquire an OpenGL context.
    CtxtMgr.releaseCurrentContext();

    pe = new PipelineEvaluator(sliceVis);
    pe->start();
    painter->start();

    app->run();

    pe->stop();
    painter->stop();

    sliceVis->deinit();
    painter->deinit();
    tgt::deinitGL();
    tgt::QtContextManager::deinit();
    tgt::deinit();

    delete painter;
    delete sliceVis;
    delete app;

    return 0;
}
