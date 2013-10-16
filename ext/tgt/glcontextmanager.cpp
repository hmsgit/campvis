#include "glcontextmanager.h"

#include "tgt/assert.h"

namespace tgt {

    GlContextManager* GlContextManager::singletonClass_ = 0;

    GlContextManager::GlContextManager()
        : _currentContext(0)
    {
    }

    GlContextManager::~GlContextManager()
    {
        for (std::map<std::string, GLCanvas*>::iterator it = _contexts.begin(); it != _contexts.end(); ++it) {
            delete it->second;
        }
        _contexts.clear();
    }


    GLCanvas* GlContextManager::getContextByKey(const std::string& key) {
        tbb::mutex::scoped_lock lock(_localMutex);
        std::map<std::string, GLCanvas*>::iterator it = _contexts.find(key);
        if (it != _contexts.end())
            return it->second;
        else
            return 0;
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

    GlContextManager* GlContextManager::getPtr() {
        tgtAssert( singletonClass_ != 0, "singletonClass_ has not been intitialized." );
        return singletonClass_;
    }

    GlContextManager& GlContextManager::getRef() {
        tgtAssert( singletonClass_ != 0 , "singletonClass_ has not been intitialized." );
        return *singletonClass_;
    }

    bool GlContextManager::isInited() {
        return (singletonClass_ != 0);
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

    void GlContextManager::removeContext(GLCanvas* context) {
        _currentContext = 0;

        tbb::mutex::scoped_lock lock(_localMutex);
        for (std::map<std::string, GLCanvas*>::iterator it = _contexts.begin(); it != _contexts.end(); ++it) {
            if (it->second == context) {
                _contexts.erase(it);
                break;
            }
        }
    }

}
