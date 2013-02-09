#include "qtcontextmanager.h"

#include "tgt/assert.h"
#include "tgt/qt/qtglcontext.h"

namespace tgt {

    QtContextManager::QtContextManager()
        : _currentContext(0)
    {
    }

    QtContextManager::~QtContextManager()
    {
        for (std::map<std::string, QtThreadedCanvas*>::iterator it = _contexts.begin(); it != _contexts.end(); ++it) {
            delete it->second;
        }
        _contexts.clear();
    }

    QtThreadedCanvas* QtContextManager::createContext(const std::string& key, const std::string& title /*= ""*/, const ivec2& size /*= ivec2(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT)*/, const GLCanvas::Buffers buffers /*= RGBADD*/, QWidget* parent /*= 0*/, bool shared /*= true*/, Qt::WFlags f /*= 0*/, char* name /*= 0*/)
    {
        tgtAssert(_contexts.find(key) == _contexts.end(), "A context with the same key already exists!");

        tbb::mutex::scoped_lock lock(_glMutex);
        QtThreadedCanvas* toReturn = new QtThreadedCanvas(title, size, buffers, parent, shared, f, name);
        _contexts.insert(std::make_pair(key, toReturn));

        toReturn->makeCurrent();
        // Init GLEW for this context
        GLenum err = glewInit();
        if (err != GLEW_OK) {
            // Problem: glewInit failed, something is seriously wrong.
            tgtAssert(false, "glewInit failed");
            std::cerr << "glewInit failed, error: " << glewGetErrorString(err) << std::endl;
            exit(EXIT_FAILURE);
        }

        return toReturn;
    }

    QtGLContext* QtContextManager::getContextByKey(const std::string& key) {
        std::map<std::string, QtThreadedCanvas*>::iterator it = _contexts.find(key);
        if (it != _contexts.end())
            return it->second->getContext();
        else
            return 0;
    }

    void QtContextManager::setCurrent(QtGLContext* context) {
        if (_currentContext != context) {
            if (context == 0) {
                // explicitely release OpenGL context
                _currentContext->getCanvas()->doneCurrent();
                _currentContext = 0;
            }
            else {
                context->getCanvas()->makeCurrent();
                LGL_ERROR;
                _currentContext = context;
            }
        }
    }

    void QtContextManager::lock() {
        _glMutex.lock();
    }

    void QtContextManager::unlock() {
        releaseCurrentContext();
        _glMutex.unlock();
    }

    tbb::mutex& QtContextManager::getGlMutex() {
        return _glMutex;
    }

    void QtContextManager::releaseCurrentContext() {
        glFlush();
        setCurrent(0);
    }

    QtCanvas* QtContextManager::getCurrentContext() const {
        return _currentContext->getCanvas();
    }


}
