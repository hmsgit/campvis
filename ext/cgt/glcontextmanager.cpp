/**********************************************************************
 *                                                                    *
 * cgt - CAMP Graphics Toolbox, Copyright (C) 2012-2015               *
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

#include "glcontextmanager.h"

#include "cgt/assert.h"
#include "cgt/logmanager.h"

namespace cgt {

    std::string GlContextManager::loggerCat_ = "cgt.GlContextManager";
    
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
        cgtAssert(context != 0, "Given context must not be 0.");
        cgtAssert(! _contextMap.find(a, context), "Tried to double register the same context.");

        ContextInfo ci = { context, new std::mutex(), std::this_thread::get_id(), true, title };
        _contextMap.insert(std::make_pair(context, ci));

        // Init GLEW for this context
        context->acquireAsCurrentContext();
        GLenum err = glewInit();
        if (err != GLEW_OK) {
            // Problem: glewInit failed, something is seriously wrong.
            cgtAssert(false, "glewInit failed");
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
            cgtAssert(false, "Could not find the context in map, this should not happen!");
        }

        _contextMap.erase(a);
    }

    std::mutex* GlContextManager::getGlMutexForContext(GLCanvas* context) {
        tbb::concurrent_hash_map<GLCanvas*, ContextInfo>::const_accessor a;
        if (_contextMap.find(a, context)) {
            return a->second._glMutex;
        }
        else {
            cgtAssert(false, "Could not find the context in map, this should not happen!");
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

                cgtAssert(a->second._acquired == false || a->second._threadId == std::this_thread::get_id(), "Tried to acquire an OpenGL thread that is already acquired by another thread!");

                ci._acquired = true;
                ci._threadId = std::this_thread::get_id();
                context->acquireAsCurrentContext();
            }
            else {
                cgtAssert(false, "Could not find the context in map, this should not happen!");
            }
        }

        tbb::spin_rw_mutex::scoped_lock lock(_threadSetMutex, true);
        _threadsWithAcquiredOpenGlContext.insert(std::this_thread::get_id());
    }

    void GlContextManager::releaseContext(GLCanvas* context, bool unlockGlMutex) {
        {
            tbb::concurrent_hash_map<GLCanvas*, ContextInfo>::const_accessor a;
            if (_contextMap.find(a, context)) {
                cgtAssert(a->second._acquired == true, "Tried to release an unbound OpenGL context!");
                cgtAssert(a->second._threadId == std::this_thread::get_id(), "Tried to release an OpenGL thread that was acquired by another thread!");
    
                ContextInfo& ci = const_cast<ContextInfo&>(a->second);
                ci._acquired = false;
                context->releaseAsCurrentContext();
    
                if (unlockGlMutex)
                    ci._glMutex->unlock();
            }
            else {
                cgtAssert(false, "Could not find the context in map, this should not happen!");
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
