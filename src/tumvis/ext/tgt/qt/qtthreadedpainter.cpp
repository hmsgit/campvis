#include "qtthreadedpainter.h"
#include "tgt/qt/qtcanvas.h"
#include "tgt/qt/qtthreadedcanvas.h"

namespace tgt {

    QtThreadedPainter::QtThreadedPainter(QtCanvas* canvas) 
        : QThread()
        , tgt::Painter(canvas)
    {
        _evaluateRenderingLoop = true;
        _viewportSizeChanged = false;
    }

    void QtThreadedPainter::stop()
    {
        _evaluateRenderingLoop = false;
    }

    void QtThreadedPainter::resizeViewport(const QSize &size)
    {
        w = size.width();
        h = size.height();
        _viewportSizeChanged = true;
    }    

    void QtThreadedPainter::run()
    {
        QtGLContext* qtContext = static_cast<QtCanvas*>(getCanvas())->getContext();
        GLContextScopedLock lock(qtContext);

        LGL_ERROR;
        while (_evaluateRenderingLoop) {
            if (_viewportSizeChanged) {
                glViewport(0, 0, w, h);
                _viewportSizeChanged = false;
            }
            paint();

            // suspend rendering thread until there is again sth. to render.
            qtContext->release();
            _renderCondition.wait(&qtContext->getRenderMutex());
            qtContext->acquire();
        }
    }

    void QtThreadedPainter::setCanvas(GLCanvas* canvas) {
        tgtAssert(dynamic_cast<QtCanvas*>(canvas) != 0, "Context must be of type QtGLContext!");
        Painter::setCanvas(canvas);
    }

}
