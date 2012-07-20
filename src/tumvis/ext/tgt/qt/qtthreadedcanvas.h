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

        virtual void setPainter(tgt::Painter* p, bool initPainter = true);

        void startRendering();
        void stopRendering();


        /**
         * This is called by the Qt framework every time the canvas is resized.
         * This function calls the corresponding GLCanvas method \a sizeChanged.
         *
         * @param w The new width of the canvas.
         * @param h The new height of the canvas.
         */
        virtual void resizeGL(int w, int h);

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

        virtual void closeEvent(QCloseEvent *evt);

    protected:

    };

}

#endif // QTTHREADEDCANVAS_H__
