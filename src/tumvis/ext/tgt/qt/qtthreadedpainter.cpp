#include "qtthreadedpainter.h"
#include "tgt/qt/qtcanvas.h"
#include "tgt/qt/qtthreadedcanvas.h"
#include "tgt/qt/qtcontextmanager.h"

namespace tgt {

    QtThreadedPainter::QtThreadedPainter(QtCanvas* canvas) 
        : QThread()
        , tgt::Painter(canvas)
    {
        _evaluateRenderingLoop = true;
    }

    void QtThreadedPainter::stop() {
        _evaluateRenderingLoop = false;
        _renderCondition.wakeAll();
    }

    void QtThreadedPainter::run() {
        QtGLContext* qtContext = static_cast<QtCanvas*>(getCanvas())->getContext();
        GLContextScopedLock lock(qtContext);

        LGL_ERROR;
        while (_evaluateRenderingLoop) {
            paint();
            glFlush();
            qtContext->getCanvas()->swap();

            /*if (qtContext->getCanvas()->doubleBuffer()) {
                if (d->autoSwap)
                    swapBuffers();
            } else {
                glFlush();
            }*/

            
            // suspend rendering thread until there is again sth. to render.
            _renderCondition.wait(&CtxtMgr.getGlMutex());
            qtContext->acquire();
        }

        // release OpenGL context, so that other threads can access it
        CtxtMgr.releaseCurrentContext();
    }

    void QtThreadedPainter::setCanvas(GLCanvas* canvas) {
        tgtAssert(dynamic_cast<QtCanvas*>(canvas) != 0, "Context must be of type QtGLContext!");
        Painter::setCanvas(canvas);
    }

}
