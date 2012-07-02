#include "tumvispainter.h"
#include "tgt/camera.h"
#include "tgt/quadric.h"

namespace TUMVis {
    const std::string TumVisPainter::loggerCat_ = "TUMVis.core.TumVisPainter";

    TumVisPainter::TumVisPainter(tgt::GLCanvas* canvas)
        : tgt::Painter(canvas) {
    }

    void TumVisPainter::paint() {
        // Put TUMVis rendering pipeline evaluation inside

        // Some Dummy rendering:
        glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);  
        getCamera()->look();  
        glColor3f(1.f, 0.f, 0.f);  
        tgt::Sphere sphere(1.f, 64, 32);  
        sphere.render();  
    }

    void TumVisPainter::sizeChanged(const tgt::ivec2& size) {
        if (getCanvas()) {
            getCanvas()->getGLFocus();
        }
        glViewport(0, 0, size.x, size.y);
    }

    void TumVisPainter::init() {
        if (getCanvas()) {
            getCanvas()->getGLFocus();
        }
        else {
            LWARNING("initialize(): No canvas");
        }

        glEnable(GL_DEPTH_TEST);  
        glShadeModel(GL_SMOOTH);  
        glEnable(GL_COLOR_MATERIAL);  
        glEnable(GL_LIGHTING);  
        glEnable(GL_LIGHT0);  

        getCamera()->setPosition(tgt::vec3(0.f,0.f,2.f)); 

/*


        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);
        glDisable(GL_CULL_FACE);
        glCullFace(GL_BACK);*/
    }
}