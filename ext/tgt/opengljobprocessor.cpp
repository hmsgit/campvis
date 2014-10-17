// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012-2014, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universitaet Muenchen
//      Boltzmannstr. 3, 85748 Garching b. Muenchen, Germany
// 
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
// 
// Licensed under the Apache License, Version 2.0 (the "License"); you may not use this file 
// except in compliance with the License. You may obtain a copy of the License at
// 
// http://www.apache.org/licenses/LICENSE-2.0
// 
// Unless required by applicable law or agreed to in writing, software distributed under the 
// License is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, 
// either express or implied. See the License for the specific language governing permissions 
// and limitations under the License.
// 
// ================================================================================================

#include "opengljobprocessor.h"

#include <tbb/tick_count.h>

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/openglgarbagecollector.h"
#include "tgt/glcontextmanager.h"

namespace tgt {

    OpenGLJobProcessor::ScopedSynchronousGlJobExecution::ScopedSynchronousGlJobExecution()
        : _lock(nullptr)
    {
        if (! GLCtxtMgr.checkWhetherThisThreadHasAcquiredOpenGlContext()) {
            GLJobProc.pause();
            _lock = new tgt::GLContextScopedLock(GLJobProc.getContext());
        }
    }

    OpenGLJobProcessor::ScopedSynchronousGlJobExecution::~ScopedSynchronousGlJobExecution() {
        if (_lock != nullptr) {
            delete _lock;
            GLJobProc.resume();
        }
    }

// ================================================================================================
    
    OpenGLJobProcessor::OpenGLJobProcessor()
    {
        _pause = 0;
        _context= nullptr;
        _performGarbageCollection = false;
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

    void OpenGLJobProcessor::stop() {
        _stopExecution = true;
        _evaluationCondition.notify_all();

        Runnable::stop();
    }

    void OpenGLJobProcessor::run() {
        tgtAssert(_context != nullptr, "You have to set the context first before calling OpenGLJobProcessor::run()!");
        std::unique_lock<std::mutex> lock(*tgt::GlContextManager::getRef().getGlMutexForContext(_context));
        tgt::GlContextManager::getRef().acquireContext(_context, false);

        while (! _stopExecution) {
            bool hadWork = false;

            AbstractJob* jobToDo = nullptr;
            while (_pause == 0 && !_stopExecution && _jobQueue.try_pop(jobToDo)) {
                hadWork = true;

                // execute and delete the job
                jobToDo->execute();
                delete jobToDo;

                performGarbageCollectionIfNecessary();
            }

            while (_pause > 0 && !_stopExecution) {
                performGarbageCollectionIfNecessary();
                tgt::GlContextManager::getRef().releaseContext(_context, false);
                _evaluationCondition.wait(lock);
                tgt::GlContextManager::getRef().acquireContext(_context, false);
                hadWork = true;
            }

            if (! hadWork && !_stopExecution) {
                performGarbageCollectionIfNecessary();
                tgt::GlContextManager::getRef().releaseContext(_context, false);
                _evaluationCondition.wait(lock);
                tgt::GlContextManager::getRef().acquireContext(_context, false);
            }
        }

        // release OpenGL context, so that other threads can access it
        tgt::GlContextManager::getRef().releaseContext(_context, false);
    }

    void OpenGLJobProcessor::pause() {
        ++_pause;
    }

    void OpenGLJobProcessor::resume() {
        if (_pause == 0) {
            tgtAssert(false, "Called resume on non-paused job processor!");
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

    void OpenGLJobProcessor::setContext(tgt::GLCanvas* context) {
        tgtAssert(_context == nullptr, "You are trying to change an already set context, thou shalt not do that!");
        _context = context;
    }

    tgt::GLCanvas* OpenGLJobProcessor::getContext() {
        return _context;
    }

    void OpenGLJobProcessor::enqueueGarbageCollection() {
        _performGarbageCollection = true;
        _evaluationCondition.notify_all();
    }

    void OpenGLJobProcessor::performGarbageCollectionIfNecessary() {
        if (_performGarbageCollection && tgt::OpenGLGarbageCollector::isInited()) {
            _performGarbageCollection = false;
            GLGC.deleteGarbage();
        }
    }


}

