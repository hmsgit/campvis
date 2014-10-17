#include "glcontextmanager.h"

#include "tgt/assert.h"
#include "tgt/logmanager.h"

namespace tgt {

    std::string GlContextManager::loggerCat_ = "tgt.GlContextManager";
    
    GlContextManager::GlContextManager() {
    }

    GlContextManager::~GlContextManager() {
        for (auto it = _contextMap.begin(); it != _contextMap.end(); ++it) {
            delete it->second._glMutex;
        }
        _contextMap.clear();
    }

    void GlContextManager::registerContextAndInitGlew(GLCanvas* context, const std::string& title) {
        tbb::concurrent_hash_map<GLCanvas*, ContextInfo>::accessor a;
        tgtAssert(context != 0, "Given context must not be 0.");
        tgtAssert(! _contextMap.find(a, context), "Tried to double register the same context.");

        ContextInfo ci = { context, new std::mutex(), std::this_thread::get_id(), true, title };
        _contextMap.insert(std::make_pair(context, ci));

        // Init GLEW for this context
        context->acquireAsCurrentContext();
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
        tbb::concurrent_hash_map<GLCanvas*, ContextInfo>::accessor a;
        if (_contextMap.find(a, context)) {
            delete a->second._glMutex;
        }
        else {
            tgtAssert(false, "Could not find the context in map, this should not happen!");
        }

        _contextMap.erase(a);
    }

    std::mutex* GlContextManager::getGlMutexForContext(GLCanvas* context) {
        tbb::concurrent_hash_map<GLCanvas*, ContextInfo>::const_accessor a;
        if (_contextMap.find(a, context)) {
            return a->second._glMutex;
        }
        else {
            tgtAssert(false, "Could not find the context in map, this should not happen!");
            return nullptr;
        }
    }

    void GlContextManager::acquireContext(GLCanvas* context, bool lockGlMutex) {
        {
            tbb::concurrent_hash_map<GLCanvas*, ContextInfo>::const_accessor a;
            if (_contextMap.find(a, context)) {
                ContextInfo& ci = const_cast<ContextInfo&>(a->second);
                if (lockGlMutex)
                    ci._glMutex->lock();

                tgtAssert(a->second._acquired == false || a->second._threadId == std::this_thread::get_id(), "Tried to acquire an OpenGL thread that is already acquired by another thread!");

                ci._acquired = true;
                ci._threadId = std::this_thread::get_id();
                context->acquireAsCurrentContext();
            }
            else {
                tgtAssert(false, "Could not find the context in map, this should not happen!");
            }
        }

        tbb::spin_rw_mutex::scoped_lock lock(_threadSetMutex, true);
        _threadsWithAcquiredOpenGlContext.insert(std::this_thread::get_id());
    }

    void GlContextManager::releaseContext(GLCanvas* context, bool unlockGlMutex) {
        {
            tbb::concurrent_hash_map<GLCanvas*, ContextInfo>::const_accessor a;
            if (_contextMap.find(a, context)) {
                tgtAssert(a->second._acquired == true, "Tried to release an unbound OpenGL context!");
                tgtAssert(a->second._threadId == std::this_thread::get_id(), "Tried to release an OpenGL thread that was acquired by another thread!");
    
                ContextInfo& ci = const_cast<ContextInfo&>(a->second);
                ci._acquired = false;
                context->releaseAsCurrentContext();
    
                if (unlockGlMutex)
                    ci._glMutex->unlock();
            }
            else {
                tgtAssert(false, "Could not find the context in map, this should not happen!");
            }
        }

        tbb::spin_rw_mutex::scoped_lock lock(_threadSetMutex, true);
        _threadsWithAcquiredOpenGlContext.erase(std::this_thread::get_id());
    }

    bool GlContextManager::checkWhetherThisThreadHasAcquiredOpenGlContext() const {
        tbb::spin_rw_mutex::scoped_lock lock(_threadSetMutex, false);
        return (_threadsWithAcquiredOpenGlContext.find(std::this_thread::get_id()) != _threadsWithAcquiredOpenGlContext.end());
    }
}
