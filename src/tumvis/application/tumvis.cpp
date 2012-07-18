#include "tgt/camera.h"
#include "tgt/exception.h"
#include "tgt/shadermanager.h"
#include "tgt/qt/qtapplication.h"
#include "tgt/qt/qtcanvas.h"

#include "tumvispainter.h"
#include "modules/pipelines/slicevis.h"


using namespace TUMVis;

/**
 * TUMVis main function, application entry point
 *
 * \param   argc    number of passed arguments
 * \param   argv    vector of arguments
 * \return  0 if program exited successfully
 **/
int main(int argc, char** argv) {  
    tgt::QtApplication* app = new tgt::QtApplication(argc, argv);
    tgt::QtCanvas* canvas = new tgt::QtCanvas("TUMVis");
    SliceVis* sliceVis = 0;

    app->addCanvas(canvas);  
    app->init();
    LogMgr.getConsoleLog()->addCat("", true);

    if (argc > 0) {
        // ugly hack
        std::string programPath(argv[0]);
        programPath = tgt::FileSystem::parentDir(tgt::FileSystem::parentDir(tgt::FileSystem::parentDir(programPath)));
        ShdrMgr.addPath(programPath);
        ShdrMgr.addPath(programPath + "/core/glsl");
    }

    tgt::Camera camera;  
    canvas->setCamera(&camera);  

    try {
        sliceVis = new SliceVis(canvas);
        canvas->setPainter(sliceVis);  
        //     TumVisPainter painter(canvas);  
        //     canvas->setPainter(&painter);  

    }
    catch (tgt::Exception& e) {
        LERRORC("main.cpp", "Encountered tgt::Exception: " << e.what());
    }
    catch (std::exception& e) {
        LERRORC("main.cpp", "Encountered std::exception: " << e.what());
    }

    app->run();

    delete sliceVis;
    delete canvas;
    delete app;  

    return 0;  
}
