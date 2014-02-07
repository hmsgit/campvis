#ifndef QTTHREADEDCANVAS_H__
#define QTTHREADEDCANVAS_H__

#include "tgt/qt/qtcanvas.h"
#include "tgt/types.h"

namespace tgt {

    /**
     * Subclass of QtCanvas that supports multi-threading.
     */
    class TGT_API QtThreadedCanvas : public tgt::QtCanvas {
    public:
        QtThreadedCanvas(
            const std::string& title = "",
            const tgt::ivec2& size = tgt::ivec2(tgt::GLCanvas::DEFAULT_WINDOW_WIDTH, tgt::GLCanvas::DEFAULT_WINDOW_HEIGHT),
            const tgt::GLCanvas::Buffers buffers = tgt::GLCanvas::RGBADD,
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
