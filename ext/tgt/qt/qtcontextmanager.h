#ifndef QTGLCONTEXTMANAGER_H__
#define QTGLCONTEXTMANAGER_H__

#include "tgt/singleton.h"
#include "tgt/glcontextmanager.h"
#include "tgt/qt/qtthreadedcanvas.h"
#include "tgt/types.h"
#include <tbb/mutex.h>  // TODO: TBB dependency in TGT is not that beautiful...
#include <map>
#include <string>

namespace tgt {
    class QtGLContext;
    class QtThreadedCanvas;

    /**
     * Manages multiple shared OpenGL contexts and offers methods to ensure that only one context is active at a time.
     * 
     */
    class TGT_API QtContextManager : public GlContextManager {
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
         * Init the actual singleton.
         * Must be called BEFORE the class is used, like this:
         */
        static void init();

        /**
         * Deinit the actual singleton.
         * Must be done at last.
         */
        static void deinit();

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
            bool shared = true);

        
    protected:
        /**
         * Sets the given context \a context as the current context for the OpenGL device.
         * If \a context is already the current context, nothing will happen.
         * \param   context     Context to set as current.
         */
        virtual void setCurrent(GLCanvas* context);

    };

} // namespace tgt


#endif // QTGLCONTEXTMANAGER_H__

