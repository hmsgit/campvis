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

#ifndef JOBPOOL_H__
#define JOBPOOL_H__

#include "sigslot/sigslot.h"
#include "tbb/include/tbb/concurrent_queue.h"

namespace TUMVis {

    class AbstractJob;

    /**
     * A JobPool manages multible Jobs in queues with different priorities.
     * 
     * \note    This class is to be considered as thread-safe.
     * \todo    Implement a suitable scheduling strategy to avoid starving of low priority jobs.
     *          This sounds like a good opportunity to take a look at the Betriebssysteme lecture slides. :)
     */
    class JobPool {
    public:
        /**
         * Enumeration of the different priorities of jobs.
         */
        enum JobPriority {
            Realtime = 0,   ///< Realtime jobs are always considered first during dequeueing.
            Normal = 1,     ///< Jobs with normal priorities are dequeued as soon as there are no realtime jobs left
            Low = 2         ///< Low priority jobs are only considered if there are no jobs in the queue with higher priority
        };

        /**
         * Creates a new JobPool
         */
        JobPool();

        /**
         * Destructor, deletes all jobs which are still enqueued.
         */
        ~JobPool();

        /**
         * Enqueues the given Job with the given priority.
         * 
         * \note    JobPool takes ownership of \a job.
         * \param job       Job to enqueue, JobPool takes ownership of this Job!
         * \param priority  Priority of the job to enqueue
         */
        void enqueueJob(AbstractJob* job, JobPriority priority);

        /**
         * Dequeues the next job according to the scheduling strategy.
         * \note    The calling function takes the ownership of the returned job!
         * \todo    Develop a good scheduling strategy and describe it here.
         * \return  The next job to execute, 0 if there is currently no job to execute. The caller takes ownership of the job!
         */
        AbstractJob* dequeueJob();

        /// Signal being emitted, when a job has been enqueued.
        sigslot::signal0<> s_enqueuedJob;

    protected:
        static const size_t NUM_PRIORITIES;             ///< total number of piorities, make sure that this matches the JobPriority enum.
        tbb::concurrent_queue<AbstractJob*>* _queues;   ///< Array of job queues, one for each JobPriority
    };
}

#endif // JOBPOOL_H__