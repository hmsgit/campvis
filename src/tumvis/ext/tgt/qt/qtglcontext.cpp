#include "qtglcontext.h"
#include "tgt/qt/qtcanvas.h"
#include "tgt/qt/qtcontextmanager.h"
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
        CtxtMgr.setCurrent(this);
    }

    void QtGLContext::lockAndAcquire() {
        CtxtMgr.lock();
        acquire();
        LGL_ERROR;
    }

    void QtGLContext::unlock() {
        CtxtMgr.unlock();
    }

    ivec2 QtGLContext::getViewportSize() const {
        return _canvas->getSize();
    }

    QtCanvas* QtGLContext::getCanvas() {
        return _canvas;
    }



}
