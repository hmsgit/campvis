// ================================================================================================
// 
// This file is part of the CAMPVis Visualization Framework.
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

#ifndef OPENGLJOBPROCESSOR_H__
#define OPENGLJOBPROCESSOR_H__

#include "sigslot/sigslot.h"
#include "tgt/singleton.h"
#include "tbb/include/tbb/atomic.h"
#include "tbb/include/tbb/concurrent_queue.h"
#include "tbb/include/tbb/concurrent_hash_map.h"
#include "tbb/include/tbb/concurrent_vector.h"
#include "tbb/include/tbb/compat/condition_variable"
#include "core/tools/job.h"
#include "core/tools/runnable.h"

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
    class OpenGLJobProcessor : public tgt::Singleton<OpenGLJobProcessor>, public Runnable, public sigslot::has_slots<> {
        friend class tgt::Singleton<OpenGLJobProcessor>;

    public:
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
         * Enqueues the given Job with the given priority.
         * 
         * \note    OpenGLJobProcessor takes ownership of \a job.
         * \param   canvas      OpenGL context for which to enqueue the job, must be registered.
         * \param   job         Job to enqueue, PriorityPool takes ownership of this Job!
         * \param   priority    Priority of the job to enqueue
         */
        void enqueueJob(tgt::GLCanvas* canvas, AbstractJob* job, JobType priority);


    protected:
        /**
         * Struct encapsulating the job queue for a single OpenGL context.
         */
        struct PerContextJobQueue {
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
                return (_serialJobs.empty() && _lowPriorityJobs.empty() && (_paintJob == 0));
            }
        };

        OpenGLJobProcessor();

        tbb::concurrent_hash_map<tgt::GLCanvas*, PerContextJobQueue*> _contextQueueMap;
        tbb::concurrent_vector<tgt::GLCanvas*> _contexts;

        std::condition_variable _evaluationCondition;   ///< conditional wait to be used when there are currently no jobs to process

        tgt::GLCanvas* _currentContext;         ///< current active OpenGL context
    };

}

#define GLJobProc tgt::Singleton<OpenGLJobProcessor>::getRef()

#endif // OPENGLJOBPROCESSOR_H__
