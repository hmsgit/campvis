/**********************************************************************
 *                                                                    *
 * cgt - CAMP Graphics Toolbox, Copyright (C) 2012-2014               *
 *     Chair for Computer Aided Medical Procedures                    *
 *     Technische Universitaet Muenchen, Germany.                     *
 *     <http://campar.in.tum.de/>                                     *
 *                                                                    *
 * forked from tgt - Tiny Graphics Toolbox, Copyright (C) 2006-2011   *
 *     Visualization and Computer Graphics Group, Department of       *
 *     Computer Science, University of Muenster, Germany.             *
 *     <http://viscg.uni-muenster.de>                                 *
 *                                                                    *
 * This file is part of the cgt library. This library is free         *
 * software; you can redistribute it and/or modify it under the terms *
 * of the GNU Lesser General Public License version 2.1 as published  *
 * by the Free Software Foundation.                                   *
 *                                                                    *
 * This library is distributed in the hope that it will be useful,    *
 * but WITHOUT ANY WARRANTY; without even the implied warranty of     *
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the       *
 * GNU Lesser General Public License for more details.                *
 *                                                                    *
 * You should have received a copy of the GNU Lesser General Public   *
 * License in the file "LICENSE.txt" along with this library.         *
 * If not, see <http://www.gnu.org/licenses/>.                        *
 *                                                                    *
 **********************************************************************/

#ifndef GLCONTEXTMANAGER_H__
#define GLCONTEXTMANAGER_H__

#include "cgt/singleton.h"
#include "cgt/glcanvas.h"
#include "cgt/types.h"

#include <ext/threading.h>

#include <tbb/concurrent_hash_map.h>
#include <tbb/spin_rw_mutex.h>

#include <set>
#include <string>

namespace cgt {
    class GLCanvas;

    /**
     * Manages multiple shared OpenGL contexts and offers methods to ensure that only one context is active at a time.
     */
    class CGT_API GlContextManager : public Singleton<GlContextManager> {
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
        void registerContextAndInitGlew(GLCanvas* context, const std::string& title);

        /**
         * Removes the OpenGL context \a context from the list of managed contexts.
         * \param   context Context to remove.
         */
        void removeContext(GLCanvas* context);

        /**
         * Returns the mutex protecting the OpenGL context for the given context
         * \param   context     OpenGL context to release.
         * \return  _glMutex
         */
        std::mutex* getGlMutexForContext(GLCanvas* context);

        /**
         * Acquires \a context as current OpenGL context.
         * \param   context     OpenGL context to acquire.
         * \param   lockGlMutex Flag whether to lock the OpenGL mutex of the context.
         */
        void acquireContext(GLCanvas* context, bool lockGlMutex);

        /**
         * Releases the given OpenGL context.
         * \param   context         OpenGL context to release.
         * \param   unlockGlMutex   Flag whether to unlock the OpenGL mutex of the context.
         */
        void releaseContext(GLCanvas* context, bool unlockGlMutex);

        /**
         * Checks whether the calling thread as an OpenGL context acquired.
         * \return  True if the calling thread as an OpenGL context acquired.
         */
        bool checkWhetherThisThreadHasAcquiredOpenGlContext() const;
        
    protected:
        /// Struct encapsulating all necessary information to manage an OpenGL thread
        struct ContextInfo {
            GLCanvas* _context;             ///< The context itself
            std::mutex* _glMutex;           ///< The mutex protecting the context from concurrent access
            std::thread::id _threadId;      ///< The thread id that acquired the context the last time (more for debug reasons)
            bool _acquired;                 ///< (Debug) flag whether this context is currently acquired
            std::string _title;             ///< Title of this context (as debug info)
        };

        /// Map of all managed OpenGL contexts
        tbb::concurrent_hash_map<GLCanvas*, ContextInfo> _contextMap;

        /// Set of all threads with currently acquired OpenGL context.
        std::set<std::thread::id> _threadsWithAcquiredOpenGlContext;
        /// Mutex protecting _threadsWithAcquiredOpenGlContext
        mutable tbb::spin_rw_mutex _threadSetMutex;

        static std::string loggerCat_;
    };


    /**
     * Scoped lockAndAcquire for a GLContext, that automatically unlocks the context on destruction.
     */
    class CGT_API GLContextScopedLock {
    public:
        explicit GLContextScopedLock(GLCanvas* context)
            : _context(context)
        {
            if (_context) {
                GlContextManager::getRef().acquireContext(context, true);
            }
        };
        ~GLContextScopedLock() {
            if (_context)
                GlContextManager::getRef().releaseContext(_context, true);
        }
    private:
        GLCanvas* _context;
    };

} // namespace cgt

#define GLCtxtMgr cgt::Singleton<cgt::GlContextManager>::getRef()


#endif // GLCONTEXTMANAGER_H__
