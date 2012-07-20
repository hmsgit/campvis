#ifndef QTGLCONTEXT_H__
#define QTGLCONTEXT_H__

#include "tgt/glcontext.h"
#include <qmutex.h>

namespace tgt {
    class QtCanvas;

    /**
     * Manages the OpenGL context of a QtCanvas and offers methods for thread-safe access.
     */
    class QtGLContext : public GLContext {
    public:
        /**
         * Creates a new QtGLContext for the OpenGL context of \a canvas.
         * \param   canvas  QtCanvas with the OpenGL context to manage
         */
        QtGLContext(QtCanvas* canvas);

        /**
         * Destructor
         */
        ~QtGLContext();

        /**
         * Acquires the OpenGL context.
         */
        virtual void acquire();

        /**
         * Releases the OpenGL context.
         */
        virtual void release();

        /**
         * Acquires the OpenGL context of the canvas and locks it for thread-safety
         * \note    Do not forget to call releaseAndUnlock() when finished rendering!
         */
        virtual void lockAndAcquire();

        /**
         * Releases and unlocks the OpenGL context of the canvas.
         */
        virtual void releaseAndUnlock();

        /**
         * Returns the viewport size of the managed OpenGL context.
         * \return  _canvas->getSize()
         */
        virtual ivec2 getViewportSize() const;

        /**
         * Returns the mutex protecting the context for multi-threaded access.
         * \return  _renderMutex
         */
        QMutex& getRenderMutex();

    protected:
        QtCanvas* _canvas;          ///< QtCanvas with the OpenGL context to manage
        QMutex _renderMutex;        ///< Mutex protecting the context for multi-threaded access
    };

} // namespace tgt

#endif // QTGLCONTEXT_H__
