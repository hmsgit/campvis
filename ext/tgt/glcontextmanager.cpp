#include "glcontextmanager.h"

#include "tgt/assert.h"

namespace tgt {
    
    GlContextManager::GlContextManager()
        : _currentContext(0)
    {
    }

    GlContextManager::~GlContextManager()
    {
        for (std::set<GLCanvas*>::iterator it = _contexts.begin(); it != _contexts.end(); ++it) {
            delete *it;
        }
        _contexts.clear();
    }

    void GlContextManager::lock() {
        _glMutex.lock();
    }

    void GlContextManager::unlock() {
        _glMutex.unlock();
    }

    tbb::mutex& GlContextManager::getGlMutex() {
        return _glMutex;
    }

    void GlContextManager::releaseCurrentContext() {
        if (_currentContext != 0)
            glFinish();
        setCurrent(0);
    }

    GLCanvas* GlContextManager::getCurrentContext() const {
        return _currentContext;
    }

    void GlContextManager::lockAndAcquire(GLCanvas* context) {
        lock();
        setCurrent(context);
    }

    void GlContextManager::releaseAndUnlock() {
        releaseCurrentContext();
        unlock();
    }

    void GlContextManager::acquireContext(GLCanvas* context) {
        setCurrent(context);
    }

    void GlContextManager::registerContextAndInitGlew(GLCanvas* context) {
        tgtAssert(context != 0, "Given context must not be 0.");
        tgtAssert(_contexts.find(context) == _contexts.end(), "Tried to double register the same context.");

        {
            tbb::mutex::scoped_lock localLock(_localMutex);
            _contexts.insert(context);
        }
       
        // Init GLEW for this context
        GLenum err = glewInit();
        if (err != GLEW_OK) {
            // Problem: glewInit failed, something is seriously wrong.
            tgtAssert(false, "glewInit failed");
            std::cerr << "glewInit failed, error: " << glewGetErrorString(err) << std::endl;
            exit(EXIT_FAILURE);
        }

        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        glPixelStorei(GL_PACK_ALIGNMENT, 1);
    }

    void GlContextManager::removeContext(GLCanvas* context) {
        _currentContext = 0;

        tbb::mutex::scoped_lock lock(_localMutex);
        std::set<GLCanvas*>::iterator it = _contexts.find(context);
        if (it != _contexts.end()) {
            _contexts.erase(it);
        }
    }

    void GlContextManager::setCurrent(GLCanvas* context) {
        if (_currentContext != context) {
            if (context == 0) {
                // explicitely release OpenGL context
                _currentContext->releaseAsCurrentContext();
                _currentContext = 0;
            }
            else {
                context->acquireAsCurrentContext();
                LGL_ERROR;
                _currentContext = context;
            }
        }

    }

}
