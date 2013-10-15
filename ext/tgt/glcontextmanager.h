#ifndef GLCONTEXTMANAGER_H__
#define GLCONTEXTMANAGER_H__

#include "tgt/singleton.h"
#include "tgt/glcanvas.h"
#include <tbb/mutex.h>  // TODO: TBB dependency in TGT is not that beautiful...
#include <map>
#include <string>

class QWidget;

namespace tgt {
    class GLCanvas;

    /**
     * Manages multiple shared OpenGL contexts and offers methods to ensure that only one context is active at a time.
     */
    class GlContextManager : public Singleton<GlContextManager> {
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
         * Get Pointer of the actual class
         * @return Pointer of the actual class
         */
        static GlContextManager* getPtr();

        /**
         * Get reference of the actual class
         * @return reference of the actual class
        */
        static GlContextManager& getRef();

        /**
         * Has the actual class been inited?
         */
        static bool isInited();

        /**
         * Creates a new OpenGL context in a QtThreadedCanvas with the given arguments.
         * Parameters are the same as in QtThreadedCanvas() but context sharing is enables per default.
         * 
         * \note    Must be called with the OpenGL mutex acquired!
         * 
         * \note    The created canvas/context is owned by this ContextManager. Hence, you may not
         *          delete it yourself!
         * 
         * \param   key     Key of the context to create, must be unique.
         * \param   title   Window title
         * \param   size    Window size
         * \return  The newly created QtThreadedCanvas.
         */
        virtual GLCanvas* createContext(
            const std::string& key,
            const std::string& title = "",
            const ivec2& size = ivec2(GLCanvas::DEFAULT_WINDOW_WIDTH, GLCanvas::DEFAULT_WINDOW_HEIGHT),
            const GLCanvas::Buffers buffers = GLCanvas::RGBADD,
            bool shared = true) = 0;
        
        /**
         * Returns the OpenGL context with the given key \a key, 0 if no such context exists.
         * \param   key     Key of the context to return.
         * \return  The OpenGL context with the given key \a key, 0 if no such context exists.
         */
        GLCanvas* getContextByKey(const std::string& key);

        GLCanvas* getCurrentContext() const;

        tbb::mutex& getGlMutex();

        void acquireContext(GLCanvas* context);

        void releaseCurrentContext();


        void lockAndAcquire(GLCanvas* context);

        void releaseAndUnlock();
        
    protected:
        /**
         * Sets the given context \a context as the current context for the OpenGL device.
         * If \a context is already the current context, nothing will happen.
         * \param   context     Context to set as current.
         */
        virtual void setCurrent(GLCanvas* context) = 0;

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


        std::map<std::string, GLCanvas*> _contexts;  ///< Map of all OpenGL contexts
        GLCanvas* _currentContext;                   ///< Current active OpenGL context
        tbb::mutex _glMutex;                                ///< Mutex protecting OpenGL for multi-threaded access

        static GlContextManager* singletonClass_;
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
