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

#include "opengljobprocessor.h"

#include <tbb/tick_count.h>

#include "cgt/assert.h"
#include "cgt/logmanager.h"
#include "cgt/glcontextmanager.h"

namespace cgt {

    OpenGLJobProcessor::OpenGLJobProcessor()
    {
        _pause = 0;
        _context= nullptr;
    }

    OpenGLJobProcessor::~OpenGLJobProcessor() {
        // delete all unfinished jobs
        AbstractJob* jobToDo = nullptr;
        while (_jobQueue.try_pop(jobToDo)) {
            delete jobToDo;
        }

        _jobQueue.clear();
        _jobPool.recycle();
    }

    void OpenGLJobProcessor::run() {
        cgtAssert(_context != nullptr, "You have to set the context first before calling OpenGLJobProcessor::run()!");
        std::unique_lock<std::mutex> lock(*cgt::GlContextManager::getRef().getGlMutexForContext(_context));
        cgt::GlContextManager::getRef().acquireContext(_context, false);

        while (! _stopExecution) {
            bool hadWork = false;

            AbstractJob* jobToDo = nullptr;
            while (_pause == 0 && !_stopExecution && _jobQueue.try_pop(jobToDo)) {
                hadWork = true;

                // execute and delete the job
                jobToDo->execute();
                delete jobToDo;
            }

            while (_pause > 0 && !_stopExecution) {
                cgt::GlContextManager::getRef().releaseContext(_context, false);
                _evaluationCondition.wait(lock);
                cgt::GlContextManager::getRef().acquireContext(_context, false);
                hadWork = true;
            }

            if (! hadWork && !_stopExecution) {
                cgt::GlContextManager::getRef().releaseContext(_context, false);
                _evaluationCondition.wait(lock);
                cgt::GlContextManager::getRef().acquireContext(_context, false);
            }
        }

        // release OpenGL context, so that other threads can access it
        cgt::GlContextManager::getRef().releaseContext(_context, false);
    }

    void OpenGLJobProcessor::pause() {
        ++_pause;
    }

    void OpenGLJobProcessor::resume() {
        if (_pause == 0) {
            cgtAssert(false, "Called resume on non-paused job processor!");
            return;
        }

        --_pause;
        if (_pause == 0)
            _evaluationCondition.notify_all();
    }

    void OpenGLJobProcessor::enqueueJob(AbstractJob* job) {
        _jobQueue.push(job);
        _evaluationCondition.notify_all();
    }

    void OpenGLJobProcessor::setContext(cgt::GLCanvas* context) {
        cgtAssert(_context == nullptr, "You are trying to change an already set context, thou shalt not do that!");
        _context = context;
    }

    cgt::GLCanvas* OpenGLJobProcessor::getContext() {
        return _context;
    }

    void OpenGLJobProcessor::enqueueJobBlocking(std::function<void(void)> fn) {
        char signalVariable = 0;
        enqueueJob(cgt::makeJobOnHeap([&signalVariable, fn]() { fn(); signalVariable = 1; }));

        while (signalVariable == 0)
            std::this_thread::yield();
    }

}

