#include "qtcontextmanager.h"

#include "tgt/assert.h"

namespace tgt {

    QtContextManager::QtContextManager()
        : GlContextManager()
    {
    }

    QtContextManager::~QtContextManager()
    {
    }

    GLCanvas* QtContextManager::createContext(const std::string& key, const std::string& title /*= ""*/, const ivec2& size /*= ivec2(DEFAULT_WINDOW_WIDTH, DEFAULT_WINDOW_HEIGHT)*/, const GLCanvas::Buffers buffers /*= RGBADD*/, bool shared /*= true*/)
    {
        // FIXME: rethink this concept of unique IDs
        //tgtAssert(_contexts.find(key) == _contexts.end(), "A context with the same key already exists!");

        tbb::mutex::scoped_lock localLock(_localMutex);
        tbb::mutex::scoped_lock lock(_glMutex);
        QtThreadedCanvas* toReturn = new QtThreadedCanvas(title, size, buffers, 0, shared);
        _contexts.insert(std::make_pair(key, toReturn));

        toReturn->makeCurrent();
        _currentContext = toReturn;
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

        releaseCurrentContext();

        return toReturn;
    }

    void QtContextManager::setCurrent(GLCanvas* context) {
        if (_currentContext != context) {
            if (context == 0) {
                // explicitely release OpenGL context
                static_cast<QtThreadedCanvas*>(_currentContext)->doneCurrent();
                _currentContext = 0;
            }
            else {
                static_cast<QtThreadedCanvas*>(context)->makeCurrent();
                LGL_ERROR;
                _currentContext = context;
            }
        }
    }

    void QtContextManager::init() {
        tgtAssert( !singletonClass_, "singletonClass_ has already been initialized." );
        singletonClass_ = new QtContextManager();
    }

    void QtContextManager::deinit() {
        tgtAssert( singletonClass_ != 0, "singletonClass_ has already been deinitialized." );
        delete singletonClass_;
        singletonClass_ = 0;
    }


}
