#ifndef QTTHREADEDCANVAS_H__
#define QTTHREADEDCANVAS_H__

#include "cgt/qt/qtcanvas.h"
#include "cgt/types.h"

namespace cgt {

    /**
     * Subclass of QtCanvas that supports multi-threading.
     */
    class CGT_API QtThreadedCanvas : public cgt::QtCanvas {
    public:
        QtThreadedCanvas(
            const std::string& title = "",
            const cgt::ivec2& size = cgt::ivec2(cgt::GLCanvas::DEFAULT_WINDOW_WIDTH, cgt::GLCanvas::DEFAULT_WINDOW_HEIGHT),
            const cgt::GLCanvas::Buffers buffers = cgt::GLCanvas::RGBADD,
            QWidget* parent = 0, bool shared = true, Qt::WFlags f = 0, char* name = 0);

        virtual ~QtThreadedCanvas();

        // override Qt events so that they don't interfere with the threading.
        void resizeEvent(QResizeEvent *event) {
            sizeChanged(ivec2(event->size().width(), event->size().height()));
        }

        /**
         * If you manually want to cause a paint-event, use this function. It will call paintGL()
         * via updateGL(). This will cause immediate repainting.
         */
        virtual void repaint();

    protected:
        // override Qt events so that they don't interfere with the threading.
        virtual void paintEvent(QPaintEvent *event);

        // override the paint method so that it doesn't interfere with the threading.
        virtual void paint();

    };

}

#endif // QTTHREADEDCANVAS_H__
