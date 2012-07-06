#include "tgt/camera.h"
#include "tgt/qt/qtapplication.h"
#include "tgt/qt/qtcanvas.h"

#include "tumvispainter.h"

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

    app->addCanvas(canvas);  
    app->init();
    

    tgt::Camera camera;  
    canvas->setCamera(&camera);  
    TumVisPainter painter(canvas);  
    canvas->setPainter(&painter);  

    app->run();  

    delete canvas;  
    delete app;  

    return 0;  
}
