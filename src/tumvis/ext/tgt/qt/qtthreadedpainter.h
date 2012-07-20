#ifndef QTTHREADEDPAINTER_H__
#define QTTHREADEDPAINTER_H__

#include "tgt/painter.h"
#include <qthread.h>
#include <qsize.h>
#include <qwaitcondition.h>

namespace tgt {
    class QtCanvas;

    /**
     * Implementation of a tgt::Painter that runs in its own thread.
     */
    class QtThreadedPainter : public QThread, public tgt::Painter {
    public:
        QtThreadedPainter(QtCanvas* canvas);
        void resizeViewport(const QSize &size);
        void run();
        void stop();

        virtual void paint() = 0;

        /// Set the context in which painter will draw
        virtual void setCanvas(GLCanvas* canvas);

    protected:
        bool _evaluateRenderingLoop;
        bool _viewportSizeChanged;
        int w;
        int h;
        int rotAngle;
        QWaitCondition _renderCondition;
    };

}

#endif // QTTHREADEDPAINTER_H__
