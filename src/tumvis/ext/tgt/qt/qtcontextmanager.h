#ifndef QTGLCONTEXTMANAGER_H__
#define QTGLCONTEXTMANAGER_H__

#include "tgt/singleton.h"
#include "tgt/qt/qtcanvas.h"
#include "tgt/qt/qtglcontext.h"
#include <qmutex.h>
#include <map>
#include <string>

namespace tgt {

    /**
     * Manages multiple shared OpenGL contexts and offers methods to ensure that only one context is active at a time.
     * 
     */
    class TGT_API QtContextManager : public Singleton<QtContextManager> {
    friend class QtGLContext;

    public:
        /**
         * Creates a new QtGLContext for the OpenGL context of \a canvas.
         * \param   canvas  QtCanvas with the OpenGL context to manage
         */
        QtContextManager();

        /**
         * Destructor
         */
        ~QtContextManager();


        /**
         * Creates a new OpenGL context in a QtCanvas with the given arguments.
         * Parameters are the same as in QtCanvas() but context sharing is enables per default.
         * The newly created context will be active, but the OpenGL mutex not be locked.
         * 
         * \note    The created canvas/context is owned by this ContextManager. Hence, you may not
         *          delete it yourself!
         * 
         * \param   key     Key of the context to create, must be unique.
         * \param   title   Window title
         * \param   size    Window size
         * \return  The newly created QtCanvas.
         */
        QtCanvas* createContext(
            const std::string& key,
            const std::string& title = "",
            const ivec2& size = ivec2(GLCanvas::DEFAULT_WINDOW_WIDTH, GLCanvas::DEFAULT_WINDOW_HEIGHT),
            const GLCanvas::Buffers buffers = GLCanvas::RGBADD,
            QWidget* parent = 0, bool shared = true, Qt::WFlags f = 0, char* name = 0);
        
        /**
         * Returns the OpenGL context with the given key \a key, 0 if no such context exists.
         * \param   key     Key of the context to return.
         * \return  The OpenGL context with the given key \a key, 0 if no such context exists.
         */
        QtGLContext* getContextByKey(const std::string& key);

        QMutex& getGlMutex();

        void releaseCurrentContext();
        
    protected:
        /**
         * Sets the given context \a context as the current context for the OpenGL device.
         * If \a context is already the current context, nothing will happen.
         * \param   context     Context to set as current.
         */
        void setCurrent(QtGLContext* context);

        /**
         * Locks the OpenGL device for other threads acessing the ContextManager.
         * \see QtContextManager::release
         */
        void lock();

        /**
         * Releases the lock on the OpenGL device
         * \see QtContextManager::lock
         */
        void unlock();


        std::map<std::string, QtCanvas*> _contexts;  ///< Map of all OpenGL contexts
        QtGLContext* _currentContext;                   ///< Current active OpenGL context
        QMutex _glMutex;                                ///< Mutex protecting OpenGL for multi-threaded access
    };

} // namespace tgt

#define CtxtMgr tgt::Singleton<tgt::QtContextManager>::getRef()

#endif // QTGLCONTEXTMANAGER_H__

