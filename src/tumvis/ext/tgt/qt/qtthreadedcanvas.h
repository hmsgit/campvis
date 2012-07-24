#ifndef QTTHREADEDCANVAS_H__
#define QTTHREADEDCANVAS_H__

#include "tgt/qt/qtcanvas.h"
#include <qmutex.h>
#include <qwaitcondition.h>

namespace tgt {

    /**
     * Subclass of QtCanvas that manages a separate rendering thread.
     */
    class QtThreadedCanvas : public tgt::QtCanvas {
    public:
        QtThreadedCanvas(
            const std::string& title = "",
            const tgt::ivec2& size = tgt::ivec2(tgt::GLCanvas::DEFAULT_WINDOW_WIDTH, tgt::GLCanvas::DEFAULT_WINDOW_HEIGHT),
            const tgt::GLCanvas::Buffers buffers = tgt::GLCanvas::RGBADD,
            QWidget* parent = 0, bool shared = false, Qt::WFlags f = 0, char* name = 0);

        ~QtThreadedCanvas();

        // override Qt events so that they don't interfere with the threading.
        void resizeEvent(QResizeEvent *event) {
            sizeChanged(ivec2(event->size().width(), event->size().height()));
        };

        // override Qt events so that they don't interfere with the threading.
        void paintEvent(QPaintEvent *event) { };

        /**
         * Called by Qt if there is a paint event; it uses the \a painter_ to paint() something.
         */
        virtual void paintGL();
        /**
         * Called by Qt if there is a paint event; it uses the \a painter_ to paint() something.
         */
        virtual void paint();

        /**
         * If you manually want to cause a paint-event, use this function. It will call paintGL()
         * via updateGL(). This will cause immediate repainting.
         */
        virtual void repaint();

    protected:

    };

}

#endif // QTTHREADEDCANVAS_H__
