#include "qtthreadedcanvas.h"
#include "cgt/assert.h"
#include "cgt/painter.h"

namespace cgt {
    
    QtThreadedCanvas::QtThreadedCanvas(const std::string& title /*= ""*/, const ivec2& size /*= ivec2(GLCanvas::DEFAULT_WINDOW_WIDTH, GLCanvas::DEFAULT_WINDOW_HEIGHT)*/, const GLCanvas::Buffers buffers /*= GLCanvas::RGBADD*/, QWidget* parent /*= 0*/, bool shared /*= false*/, Qt::WFlags f /*= 0*/, char* name /*= 0*/)
        : QtCanvas(title, size, buffers, parent, shared, f, name)
    {
    }

    QtThreadedCanvas::~QtThreadedCanvas() {

    }

    void QtThreadedCanvas::repaint() {
        // skip QtCanvas's repaint implementation and generate a paint event with QWidget's repaint instead
        QWidget::repaint();
    }

    void QtThreadedCanvas::paint() {
        // all painting done in threaded painter
    }

    void QtThreadedCanvas::paintEvent(QPaintEvent* /* event */) {
        painter_->repaint();
    }

}
