// ================================================================================================
// 
// This file is part of the TUMVis Visualization Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge (christian.szb@in.tum.de)
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// 
// The licensing of this softare is not yet resolved. Until then, redistribution in source or
// binary forms outside the CAMP chair is not permitted, unless explicitly stated in legal form.
// However, the names of the original authors and the above copyright notice must retain in its
// original state in any case.
// 
// Legal disclaimer provided by the BSD license:
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND ANY EXPRESS OR
// IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY 
// AND FITNESS FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT HOLDER OR 
// CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR 
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR 
// SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON ANY 
// THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR 
// OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
// POSSIBILITY OF SUCH DAMAGE.
// 
// ================================================================================================

#include "opengljobprocessor.h"

#include "tgt/assert.h"
#include "tgt/qt/qtcontextmanager.h"
#include "core/tools/job.h"

namespace TUMVis {

    OpenGLJobProcessor::OpenGLJobProcessor()
        : _currentContext(0)
        , _startTimeCurrentContext(0)
    {
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
    }

    void OpenGLJobProcessor::stop() {
        _stopExecution = true;
        _evaluationCondition.notify_all();

        Runnable::stop();
    }

    void OpenGLJobProcessor::run() {
        std::unique_lock<tbb::mutex> lock(CtxtMgr.getGlMutex());

        while (! _stopExecution) {
            // this is a simple round-robing scheduling between all contexts:
            bool hadWork = false;
            // TODO: consider only non-empty context queues here
            clock_t maxTimePerContext = 30 / _contexts.size();

            for (size_t i = 0; i < _contexts.size(); ++i) {
                _startTimeCurrentContext = clock() * 1000 / CLOCKS_PER_SEC;
                tgt::GLCanvas* context = _contexts[i];

                tbb::concurrent_hash_map<tgt::GLCanvas*, PerContextJobQueue*>::const_accessor a;
                if (!_contextQueueMap.find(a, context)) {
                    tgtAssert(false, "Should not reach this: Did not find context in contextQueueMap!");
                    break;
                }

                // avoid expensive context-switches for contexts without oending jobs.
                if (a->second->empty())
                    continue;

                // perform context switch if necessary
                if (_currentContext != context) {
                    if (_currentContext != 0) {
                        glFinish();
                        LGL_ERROR;
                    }
                    context->getContext()->acquire();
                    _currentContext = context;
                }

                // now comes the per-context scheduling strategy:
                // first: perform as much serial jobs as possible:
                AbstractJob* jobToDo = 0;
                while ((clock() * 1000 / CLOCKS_PER_SEC) - _startTimeCurrentContext < maxTimePerContext) {
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
                if ((jobToDo = a->second->_paintJob) != 0) {
                    jobToDo->execute();
                    delete jobToDo;
                    a->second->_paintJob = 0;
                }

                // update hadWork flag
                hadWork = (jobToDo != 0);
            }

            if (! hadWork) {
                CtxtMgr.releaseCurrentContext();
                _evaluationCondition.wait(lock);
                _currentContext->getContext()->acquire();
            }
        }

        // release OpenGL context, so that other threads can access it
        CtxtMgr.releaseCurrentContext();
    }

    void OpenGLJobProcessor::enqueueJob(tgt::GLCanvas* canvas, AbstractJob* job, JobType priority) {
        tbb::concurrent_hash_map<tgt::GLCanvas*, PerContextJobQueue*>::const_accessor a;
        if (_contextQueueMap.find(a, canvas)) {
            switch (priority) {
            case PaintJob:
                if (a->second->_paintJob != 0)
                    delete a->second->_paintJob;
                a->second->_paintJob = job;
                break;
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
            tgtAssert(false, "Specified context not found. Context must be registered before they can have jobs.");
        }

        _evaluationCondition.notify_all();
    }

    void OpenGLJobProcessor::registerContext(tgt::GLCanvas* context) {
#ifdef TUMVIS_DEBUG
        tbb::concurrent_hash_map<tgt::GLCanvas*, PerContextJobQueue*>::const_accessor a;
        if (_contextQueueMap.find(a, context))
            tgtAssert(false, "Contexts shall only be registered once!");
#endif

        PerContextJobQueue* newQueue = new PerContextJobQueue;
        _contextQueueMap.insert(std::make_pair(context, newQueue));
        _contexts.push_back(context);
    }

}

