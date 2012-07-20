#include "qtthreadedcanvas.h"
#include "tgt/assert.h"
#include "tgt/qt/qtthreadedpainter.h"

namespace tgt {
    
    QtThreadedCanvas::QtThreadedCanvas(const std::string& title /*= ""*/, const ivec2& size /*= ivec2(GLCanvas::DEFAULT_WINDOW_WIDTH, GLCanvas::DEFAULT_WINDOW_HEIGHT)*/, const GLCanvas::Buffers buffers /*= GLCanvas::RGBADD*/, QWidget* parent /*= 0*/, bool shared /*= false*/, Qt::WFlags f /*= 0*/, char* name /*= 0*/)
        : QtCanvas(title, size, buffers, parent, shared, f, name)
    {
    }

    

    QtThreadedCanvas::~QtThreadedCanvas() {

    }

    void QtThreadedCanvas::startRendering() {
        if (painter_ != 0) {
            QtThreadedPainter* painter = static_cast<QtThreadedPainter*>(painter_);
            painter->start();
        }
    }

    void QtThreadedCanvas::stopRendering() {
        if (painter_ != 0) {
            QtThreadedPainter* painter = static_cast<QtThreadedPainter*>(painter_);
            painter->stop();
            painter->wait();
        }
    }

    void QtThreadedCanvas::resizeGL(int w, int h) {
        if (painter_ != 0) {
            painter_->sizeChanged(ivec2(w, h));
        }
    }

    void QtThreadedCanvas::paintGL() {
        // all painting done in QtThreadedPainter
    }

    void QtThreadedCanvas::repaint() {
        // all painting done in QtThreadedPainter
    }

    void QtThreadedCanvas::closeEvent(QCloseEvent *evt) {
        stopRendering();
        QtCanvas::closeEvent(evt);
    }

    void QtThreadedCanvas::paint() {
        // all painting done in QtThreadedPainter
    }

    void QtThreadedCanvas::setPainter(Painter* p, bool initPainter /* = true */) {
        tgtAssert(dynamic_cast<QtThreadedPainter*>(p) != 0, "Painter must be of type QtThreadedPainter!");
        GLCanvas::setPainter(p, initPainter);
    }

}
