#ifndef GLCONTEXTMANAGER_H__
#define GLCONTEXTMANAGER_H__

#include "tgt/singleton.h"
#include "tgt/glcanvas.h"
#include "tgt/types.h"

#include <ext/threading.h>
#include <set>
#include <string>

namespace tgt {
    class GLCanvas;

    /**
     * Manages multiple shared OpenGL contexts and offers methods to ensure that only one context is active at a time.
     */
    class TGT_API GlContextManager : public Singleton<GlContextManager> {
    public:
        /**
         * Creates a new QtGLContext for the OpenGL context of \a canvas.
         * \param   canvas  QtCanvas with the OpenGL context to manage
         */
        GlContextManager();

        /**
         * Destructor
         */
        virtual ~GlContextManager();

        /**
         * Registers \a canvas as new managed OpenGL context and initializes GLEW.
         * \param   canvas  OpenGL context to register
         */
        void registerContextAndInitGlew(GLCanvas* context);

        /**
         * Removes the OpenGL context \a context from the list of managed contexts.
         * \param   context Context to remove.
         */
        void removeContext(GLCanvas* context);
        
        /**
         * Returns the currently active OpenGL context.
         * \return  _currentContext
         */
        GLCanvas* getCurrentContext() const;

        /**
         * Returns the mutex protecting the OpenGL context
         * \return  _glMutex
         */
        std::mutex& getGlMutex();

        /**
         * Acquires \a context as current OpenGL context.
         * \param   context     OpenGL context to acquire.
         */
        void acquireContext(GLCanvas* context);

        /**
         * Releases the currently bound OpenGL context.
         */
        void releaseCurrentContext();


        /**
         * Locks the OpenGL context mutex and acquires \a context as current OpenGl context.
         * \param   context     OpenGL context to acquire.
         */
        void lockAndAcquire(GLCanvas* context);

        /**
         * Releases the currently bound OpenGL context and also releases the OpenGL mutex.
         */
        void releaseAndUnlock();
        
    protected:
        /**
         * Sets the given context \a context as the current context for the OpenGL device.
         * If \a context is already the current context, nothing will happen.
         * \param   context     Context to set as current.
         */
        void setCurrent(GLCanvas* context);

        /**
         * Locks the OpenGL device for other threads acessing the ContextManager.
         * \see GlContextManager::release
         */
        void lock();

        /**
         * Releases the lock on the OpenGL device
         * \see GlContextManager::lock
         */
        void unlock();


        std::set<GLCanvas*> _contexts;  ///< Map of all OpenGL contexts
        GLCanvas* _currentContext;                   ///< Current active OpenGL context
        std::mutex _glMutex;                         ///< Mutex protecting OpenGL for multi-threaded access

        std::mutex _localMutex;                      ///< local mutex to prodect _contexts
    };


    /**
     * Scoped lockAndAcquire for a GLContext, that automatically unlocks the context on destruction.
     */
    class TGT_API GLContextScopedLock {
    public:
        GLContextScopedLock(GLCanvas* context)
            : _context(context)
        {
            if (_context) {
                GlContextManager::getRef().lockAndAcquire(context);
            }
        };
        ~GLContextScopedLock() {
            if (_context)
                GlContextManager::getRef().releaseAndUnlock();
        }
    private:
        GLCanvas* _context;
    };

} // namespace tgt


#endif // GLCONTEXTMANAGER_H__
