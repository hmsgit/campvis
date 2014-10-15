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

#include "tgt/glcontextmanager.h"
#include "tgt/job.h"
#include "tgt/runnable.h"
#include "tgt/singleton.h"
#include "tgt/singleton.h"

#include <ext/threading.h>

#define TBB_PREVIEW_MEMORY_POOL 1
#include <tbb/atomic.h>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_vector.h>
#include <tbb/memory_pool.h>


namespace tgt {
    class GLCanvas;

    /**
     * This job processor singleton can be used to execute jobs that need an OpenGL context.
     * 
     * Implementing the Runnable interface, the OpenGLJobProcessor runs a background thread having
     * and acquired OpenGL context. You can execute OpenGL calls asynchroniously using enqueueJob()
     * or synchronously using the ScopedSynchronousGlJobExecution guard.
     */
    class TGT_API OpenGLJobProcessor : public tgt::Singleton<OpenGLJobProcessor>, public tgt::Runnable, public sigslot::has_slots {
        friend class tgt::Singleton<OpenGLJobProcessor>;    ///< CRTP
        friend class AbstractJob;                           ///< so the custom new/delete operator can access the memory pool

    public:
        /**
         * Scope guard to ensure that encapsulated job is synchronously executed in an arbitrary OpenGL context.
         * This scope guard checks whether current thread is OpenGLJobProcessor thread. If so, it
         * does nothing. If this thread is not the OpenGL thread, the OpenGLJobProcessor is paused,
         * an arbitrary OpenGL context acquired. Upon destruction the OpenGLJobProcessor is resumed.
         */
        class TGT_API ScopedSynchronousGlJobExecution {
        public:
            ScopedSynchronousGlJobExecution();
            ~ScopedSynchronousGlJobExecution();

        private:
            tgt::GLContextScopedLock* _lock;
        };


        /**
         * Destructor, deletes all unfinished jobs.
         */
        virtual ~OpenGLJobProcessor();


        /**
         * Registers the given OpenGL context, so that it gets its own job queue.
         * \param   context     OpenGL context to register.
         */
        void setContext(tgt::GLCanvas* context);

        /**
         * Returns the OpenGL context of this object..
         * \return  _context
         */
        tgt::GLCanvas* getContext();

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
         * Enqueues the given job.
         * 
         * \note    OpenGLJobProcessor takes ownership of \a job.
         * \param   job         Job to enqueue, PriorityPool takes ownership of this Job!
         */
        void enqueueJob(AbstractJob* job);


    protected:
        // Protected constructor since it's a singleton
        OpenGLJobProcessor();

        tgt::GLCanvas* _context;                        ///< The OpenGL context to use
        tbb::concurrent_queue<AbstractJob*> _jobQueue;  ///< The OpenGL job queue

        tbb::atomic<int> _pause;                        ///< Counter of pause requests
        std::condition_variable _evaluationCondition;   ///< conditional wait to be used when there are currently no jobs to process

    private:
        typedef std::allocator<AbstractJob> pool_allocator_t;
        tbb::memory_pool<pool_allocator_t> _jobPool;    ///< Memory pool for the signals
    };

}

#define GLJobProc tgt::Singleton<tgt::OpenGLJobProcessor>::getRef()

#endif // OPENGLJOBPROCESSOR_H__
