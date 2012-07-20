#include "qtglcontext.h"
#include "tgt/qt/qtcanvas.h"
#include "tgt/assert.h"

namespace tgt {

    QtGLContext::QtGLContext(QtCanvas* canvas)
        : GLContext()
        , _canvas(canvas)
    {
        tgtAssert(canvas != 0, "Canvas must not be 0.");
    }

    QtGLContext::~QtGLContext() {

    }

    void QtGLContext::acquire() {
        _canvas->makeCurrent();
    }

    void QtGLContext::release() {
        _canvas->doneCurrent();
    }

    void QtGLContext::lockAndAcquire() {
        _renderMutex.lock();
        acquire();
    }

    void QtGLContext::releaseAndUnlock() {
        release();
        _renderMutex.unlock();
    }

    ivec2 QtGLContext::getViewportSize() const {
        return _canvas->getSize();
    }

    QMutex& QtGLContext::getRenderMutex() {
        return _renderMutex;
    }


}
