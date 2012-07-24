#include "qtthreadedcanvas.h"
#include "tgt/assert.h"

namespace tgt {
    
    QtThreadedCanvas::QtThreadedCanvas(const std::string& title /*= ""*/, const ivec2& size /*= ivec2(GLCanvas::DEFAULT_WINDOW_WIDTH, GLCanvas::DEFAULT_WINDOW_HEIGHT)*/, const GLCanvas::Buffers buffers /*= GLCanvas::RGBADD*/, QWidget* parent /*= 0*/, bool shared /*= false*/, Qt::WFlags f /*= 0*/, char* name /*= 0*/)
        : QtCanvas(title, size, buffers, parent, shared, f, name)
    {
    }

    QtThreadedCanvas::~QtThreadedCanvas() {

    }

    void QtThreadedCanvas::paintGL() {
        // all painting done in threaded painter
    }

    void QtThreadedCanvas::repaint() {
        // all painting done in threaded painter
    }

    void QtThreadedCanvas::paint() {
        // all painting done in threaded painter
    }

}
