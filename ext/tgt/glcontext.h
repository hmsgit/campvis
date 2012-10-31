#ifndef GLCONTEXT_H__
#define GLCONTEXT_H__

#include "tgt/vector.h"

namespace tgt {
    /**
     * Abstract base class for thread-safe OpenGL contexts.
     */
    class TGT_API GLContext {
    public:
        GLContext();
        ~GLContext();

        virtual void acquire() = 0;

        virtual void lockAndAcquire() = 0;
        virtual void unlock() = 0;

        virtual ivec2 getViewportSize() const = 0;
    };

    /**
     * Scoped lockAndAcquire for a GLContext, that automatically unlocks the context on destruction.
     */
    class TGT_API GLContextScopedLock {
    public:
        GLContextScopedLock(GLContext* context)
            : _context(context)
        {
            if (_context)
                _context->lockAndAcquire();
        };
        ~GLContextScopedLock() {
            if (_context)
                _context->unlock();
        }
    private:
        GLContext* _context;
    };

} // namespace tgt

#endif // GLCONTEXT_H__
