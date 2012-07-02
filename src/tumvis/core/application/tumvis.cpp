#include "tgt/camera.h"
#include "tgt/qt/qtapplication.h"
#include "tgt/qt/qtcanvas.h"

#include "tumvispainter.h"

namespace TUMVis {

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

}
