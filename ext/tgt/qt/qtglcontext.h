#ifndef QTGLCONTEXT_H__
#define QTGLCONTEXT_H__

#include "tgt/glcontext.h"
#include <qmutex.h>

namespace tgt {
    class QtCanvas;

    /**
     * Manages the OpenGL context of a QtCanvas and offers methods for thread-safe access.
     */
    class TGT_API QtGLContext : public GLContext {
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
         * Acquires the OpenGL context of the canvas and locks it for thread-safety
         * \note    Do not forget to call unlock() when finished rendering!
         */
        virtual void lockAndAcquire();

        /**
         * Unlocks the OpenGL context manager.
         */
        virtual void unlock();

        /**
         * Returns the viewport size of the managed OpenGL context.
         * \return  _canvas->getSize()
         */
        virtual ivec2 getViewportSize() const;

        /**
         * Returns the QtCanvas owning this context.
         * \return QtCanvas owning this context.
         */
        QtCanvas* getCanvas();

    protected:
        QtCanvas* _canvas;          ///< QtCanvas owning this context.
    };

} // namespace tgt

#endif // QTGLCONTEXT_H__
