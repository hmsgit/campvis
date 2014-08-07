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

#include "tgt/assert.h"
#include "tgt/logmanager.h"
#include "tgt/openglgarbagecollector.h"
#include "tgt/glcontextmanager.h"
#include "core/tools/job.h"

namespace campvis {

    std::thread::id OpenGLJobProcessor::_this_thread_id;

// ================================================================================================

    OpenGLJobProcessor::ScopedSynchronousGlJobExecution::ScopedSynchronousGlJobExecution()
        : _lock(nullptr)
    {
        if (! GLJobProc.isCurrentThreadOpenGlThread()) {
            GLJobProc.pause();
            _lock = new tgt::GLContextScopedLock(GLJobProc.iKnowWhatImDoingGetArbitraryContext());
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
        _currentContext = 0;
    }

    OpenGLJobProcessor::~OpenGLJobProcessor() {
        // delete all per-context job queues and unfinished jobs
        for (tbb::concurrent_vector<tgt::GLCanvas*>::const_iterator it = _contexts.begin(); it != _contexts.end(); ++it) {
            tbb::concurrent_hash_map<tgt::GLCanvas*, PerContextJobQueue*>::const_accessor a;
            if (_contextQueueMap.find(a, *it)) {
                delete a->second;
            }
        }

        _contextQueueMap.clear();
        _contexts.clear();
        _jobPool.recycle();
    }

    void OpenGLJobProcessor::stop() {
        _stopExecution = true;
        _evaluationCondition.notify_all();
    }

    void OpenGLJobProcessor::run() {
        _this_thread_id = std::this_thread::get_id();

        std::unique_lock<std::mutex> lock(tgt::GlContextManager::getRef().getGlMutex());
        clock_t lastCleanupTime = clock() * 1000 / CLOCKS_PER_SEC;

        while (! _stopExecution) {
            // this is a simple round-robing scheduling between all contexts:
            bool hadWork = false;
            // TODO: consider only non-empty context queues here
            clock_t maxTimePerContext = static_cast<clock_t>(30 / _contexts.size());

            for (size_t i = 0; i < _contexts.size(); ++i) {
                clock_t startTimeCurrentContext = clock() * 1000 / CLOCKS_PER_SEC;
                tgt::GLCanvas* context = _contexts[i];

                tbb::concurrent_hash_map<tgt::GLCanvas*, PerContextJobQueue*>::const_accessor a;
                if (!_contextQueueMap.find(a, context)) {
                    //tgtAssert(false, "Should not reach this: Did not find context in contextQueueMap!");
                    continue;
                }

                // avoid expensive context-switches for contexts without pending jobs.
                if (a->second->empty())
                    continue;

                // we will have work, so update the flag
                hadWork = true;

                // perform context switch if necessary
                if (_currentContext != context) {
                    if (_currentContext != 0) {
                        glFinish();
                        LGL_ERROR;
                    }
                    tgt::GlContextManager::getRef().acquireContext(context);
                    _currentContext = context;
                }

                // now comes the per-context scheduling strategy:
                // first: perform as much serial jobs as possible:
                AbstractJob* jobToDo = 0;
                while ((clock() * 1000 / CLOCKS_PER_SEC) - startTimeCurrentContext < maxTimePerContext) {
                    // try fetch a job
                    if (! a->second->_serialJobs.try_pop(jobToDo)) {
                        // no job to do, exit the while loop
                        break;
                    }
                    // execute and delete the job
                    jobToDo->execute();
                    delete jobToDo;
                }

                // second: execute one low-prio job if existant
                if (a->second->_lowPriorityJobs.try_pop(jobToDo)) {
                    jobToDo->execute();
                    delete jobToDo;
                }

                // third: execute paint job
                jobToDo = a->second->_paintJob.fetch_and_store(0);
                if (jobToDo != 0) {
                    jobToDo->execute();
                    delete jobToDo;
                }


                // fourth: start the GC if it's time
                if (clock() * 1000 / CLOCKS_PER_SEC - lastCleanupTime > 250) {
                    GLGC.deleteGarbage();
                    lastCleanupTime = clock();
                }
            }
            
            while (_pause > 0) {
                GLGC.deleteGarbage();
                lastCleanupTime = clock();
                tgt::GlContextManager::getRef().releaseCurrentContext();
                _evaluationCondition.wait(lock);
                tgt::GlContextManager::getRef().acquireContext(_currentContext);
                hadWork = true;
            }

            if (! hadWork) {
                if (_currentContext != 0) {
                    GLGC.deleteGarbage();
                    lastCleanupTime = clock();
                }
                tgt::GlContextManager::getRef().releaseCurrentContext();
                _evaluationCondition.wait(lock);
                tgt::GlContextManager::getRef().acquireContext(_currentContext);
            }
        }

        // release OpenGL context, so that other threads can access it
        tgt::GlContextManager::getRef().releaseCurrentContext();
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

    void OpenGLJobProcessor::enqueueJob(tgt::GLCanvas* canvas, AbstractJob* job, JobType priority) {
        tbb::concurrent_hash_map<tgt::GLCanvas*, PerContextJobQueue*>::const_accessor a;
        if (_contextQueueMap.find(a, canvas)) {
            switch (priority) {
            case PaintJob:
                {
                    AbstractJob* oldJob = a->second->_paintJob.fetch_and_store(job);
                    if (oldJob != 0)
                        delete oldJob;
                    break;
                }
            case SerialJob:
                a->second->_serialJobs.push(job);
                break;
            case LowPriorityJob:
                a->second->_lowPriorityJobs.push(job);
                break;
            default:
                tgtAssert(false, "Should not reach this - wrong job type!");
                break;
            } 
        }
        else {
            tgtAssert(false, "Specified context not found. Contexts must be registered before they can have jobs.");
        }

        _evaluationCondition.notify_all();
    }

    void OpenGLJobProcessor::registerContext(tgt::GLCanvas* context) {
#ifdef CAMPVIS_DEBUG
        tbb::concurrent_hash_map<tgt::GLCanvas*, PerContextJobQueue*>::const_accessor a;
        if (_contextQueueMap.find(a, context))
            tgtAssert(false, "Contexts shall only be registered once!");
#endif

        PerContextJobQueue* newQueue = new PerContextJobQueue;
        _contextQueueMap.insert(std::make_pair(context, newQueue));
        _contexts.push_back(context);
    }

    void OpenGLJobProcessor::deregisterContext(tgt::GLCanvas* context) {
        tbb::concurrent_hash_map<tgt::GLCanvas*, PerContextJobQueue*>::accessor a;
        if (_contextQueueMap.find(a, context)) {
            delete a->second;
            _contextQueueMap.erase(a);
        }

        if (_currentContext == context)
            _currentContext.compare_and_swap(0, context);
    }

    tgt::GLCanvas* OpenGLJobProcessor::iKnowWhatImDoingGetArbitraryContext() {
        if (_currentContext != 0)
            return _currentContext;
        else if (!_contexts.empty())
            return _contexts.front();
        else {
            tgtAssert(false, "No Contexts registered!");
            return 0;
        }
    }

    bool OpenGLJobProcessor::isCurrentThreadOpenGlThread() const {
        return std::this_thread::get_id() == _this_thread_id;
    }



}

