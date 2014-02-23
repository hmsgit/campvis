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

#ifndef OPENGLJOBPROCESSOR_H__
#define OPENGLJOBPROCESSOR_H__

#include "sigslot/sigslot.h"
#include "tgt/runnable.h"
#include "tgt/singleton.h"

#include <ext/threading.h>

#include <tbb/atomic.h>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_vector.h>

#include "tgt/glcontextmanager.h"
#include "tgt/singleton.h"

#include "core/coreapi.h"
#include "core/tools/job.h"

#include <ctime>


namespace tgt {
    class GLCanvas;
}

namespace campvis {
    /**
     * Singleton class for managing and executing work items (jobs) that need an active OpenGL context.
     * After an OpenGL context has been registered you can enqueue jobs that are to be executed within that 
     * context to the job queue. Enqueued jobs are executed asynchroniously using a specific scheduling 
     * strategy, depending on the given JobType:
     * 
     * OpenGLJobProcessor implements a round-robin scheduling strategy for the registered OpenGL contexts, 
     * meaning that each context gets roughly the same computing time. Thereby, it tries to maintain an update
     * frequency of 30fps for each context.
     * The jobs for each contexts are scheduled using the following technique: As mentioned above, each context
     * has a time slot of \a n milliseconds. At first, as much serial jobs are executed as possible until their
     * queue is empty or or the time is up. Then, one low priority job is executed. Finally, the PaintJob is
     * executed (if existant) before switching to the next context.
     * 
     * This class implements the Runnable interface, hence, runs in its own thread. Furthermore, it uses 
     * conditional wait, when there are currently no jobs to process.
     * 
     * This class is to be considered as thread-safe.
     */
    class CAMPVIS_CORE_API OpenGLJobProcessor : public tgt::Singleton<OpenGLJobProcessor>, public tgt::Runnable, public sigslot::has_slots<> {
        friend class tgt::Singleton<OpenGLJobProcessor>;

    public:
        /**
         * Scope guard to ensure that encapsulated job is synchronously executed in an arbitrary OpenGL context.
         * This scope guard checks whether current thread is OpenGLJobProcessor thread. If so, it
         * does nothing. If this thread is not the OpenGL thread, the OpenGLJobProcessor is paused,
         * an arbitrary OpenGL context acquired. Upon destruction the OpenGLJobProcessor is resumed.
         */
        class ScopedSynchronousGlJobExecution {
        public:
            ScopedSynchronousGlJobExecution();
            ~ScopedSynchronousGlJobExecution();

        private:
            tgt::GLContextScopedLock* _lock;
        };

        /**
         * Enumeration of the different priorities of items.
         */
        enum JobType {
            PaintJob,           ///< PaintJobs have the highest priority, there can only be one paint job per context at a time.
            SerialJob,          ///< SerialJobs have a lower priority than PaintJobs, but are guaranteed to be executed in order.
            LowPriorityJob      ///< Low priority jobs have the lowest priority, can be executed at any time. The only guarantee is that thay won't starve.
        };

        /**
         * Destructor, deletes all unfinished jobs.
         */
        virtual ~OpenGLJobProcessor();


        /**
         * Registers the given OpenGL context, so that it gets its own job queue.
         * \param   context     OpenGL context to register.
         */
        void registerContext(tgt::GLCanvas* context);

        /**
         * Deregisters the given OpenGL context, so that it has no longer its own job queue.
         * \param   context     OpenGL context to deregister.
         */
        void deregisterContext(tgt::GLCanvas* context);

        /// \see Runnable::stop
        void stop();
        
        /**
         * Performs the job processing using conditional wait.
         * \sa Runnable::run
         */
        void run();

        /**
         * Pauses the job processor as at the next possible moment.
         */
        void pause();

        /**
         * Resume the execution of the job processor.
         */
        void resume();

        /**
         * Enqueues the given Job with the given priority.
         * 
         * \note    OpenGLJobProcessor takes ownership of \a job.
         * \param   canvas      OpenGL context for which to enqueue the job, must be registered.
         * \param   job         Job to enqueue, PriorityPool takes ownership of this Job!
         * \param   priority    Priority of the job to enqueue
         */
        void enqueueJob(tgt::GLCanvas* canvas, AbstractJob* job, JobType priority);


        /**
         * Returns an arbitrary registered OpenGL context.
         * \note    You can do really messy things with this. Do not use this method unless
         *          you know what you're doing and know that there is no other way...
         */
        tgt::GLCanvas* iKnowWhatImDoingGetArbitraryContext();

        /**
         * Checks whether calling thread is OpenGL thread.
         * \return  std::this_thread::get_id() == _this_thread_id
         */
        bool isCurrentThreadOpenGlThread() const;

    protected:
        /**
         * Struct encapsulating the job queue for a single OpenGL context.
         */
        struct CAMPVIS_CORE_API PerContextJobQueue {
            /**
             * Creates an empty PerContextJobQueue.
             */
            PerContextJobQueue() {
                _paintJob = 0;
            }

            /**
             * Destructor, deletes all enqueued jobs.
             */
            ~PerContextJobQueue() {
                if (_paintJob != 0)
                    delete _paintJob;

                AbstractJob* jobToDelete = 0;
                while (_serialJobs.try_pop(jobToDelete))
                    delete jobToDelete;
                while (_lowPriorityJobs.try_pop(jobToDelete))
                    delete jobToDelete;
            }

            tbb::atomic<AbstractJob*> _paintJob;                    ///< PaintJob of the context
            tbb::concurrent_queue<AbstractJob*> _serialJobs;        ///< Queue of serial jobs for the context
            tbb::concurrent_queue<AbstractJob*> _lowPriorityJobs;   ///< Queue of jow priority jobs for the context

            /**
             * Checks, whether there is any job to do for this context.
             * \return (_serialJobs.empty() && _lowPriorityJobs.empty() && (_paintJob == 0))
             */
            bool empty() const {
                return ((_paintJob == 0) && _serialJobs.empty() && _lowPriorityJobs.empty());
            }
        };

        OpenGLJobProcessor();

        tbb::concurrent_hash_map<tgt::GLCanvas*, PerContextJobQueue*> _contextQueueMap;
        tbb::concurrent_vector<tgt::GLCanvas*> _contexts;

        tbb::atomic<int> _pause;
        std::condition_variable _evaluationCondition;   ///< conditional wait to be used when there are currently no jobs to process

        tbb::atomic<tgt::GLCanvas*> _currentContext;         ///< current active OpenGL context

    private:
        static std::thread::id _this_thread_id;
    };

}

#define GLJobProc tgt::Singleton<campvis::OpenGLJobProcessor>::getRef()

#endif // OPENGLJOBPROCESSOR_H__
