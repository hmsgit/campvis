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

namespace TUMVis {
    /**
     *
     * \todo    Check if the inheritance of PriorityPool is a cool design or not...
     */
    class OpenGLJobProcessor : public tgt::Singleton<OpenGLJobProcessor>, public Runnable, public sigslot::has_slots<> {
        friend class tgt::Singleton<OpenGLJobProcessor>;

    public:
        /**
         * Enumeration of the different priorities of items.
         */
        enum JobType {
            PaintJob,           ///< PaintJobs have the highest priority
            SerialJob,          ///< SerialJobs have a lower priority than PaintJobs, but are guaranteed to be executed in order.
            LowPriorityJob      ///< Low priority jobs have the lowest priority, can be executed at any time. The only guarantee is that thay won't starve.
        };

        virtual ~OpenGLJobProcessor();


        void registerContext(tgt::GLCanvas* context);

        /// \see Runnable::stop
        void stop();
        
        /**
         * Performs the pipeline evaluation using conditional wait.
         * \sa Runnable::run
         */
        void run();

        /**
         * Enqueues the given Job with the given priority.
         * 
         * \note    OpenGLJobProcessor takes ownership of \a job.
         * \param   job         Job to enqueue, PriorityPool takes ownership of this Job!
         * \param   priority    Priority of the job to enqueue
         */
        void enqueueJob(tgt::GLCanvas* canvas, AbstractJob* job, JobType priority);


    protected:
        struct PerContextJobQueue {
            PerContextJobQueue() 
                : _paintJob(0)
            {
            }

            AbstractJob* _paintJob;
            tbb::concurrent_queue<AbstractJob*> _serialJobs;
            tbb::concurrent_queue<AbstractJob*> _lowPriorityJobs;

            bool empty() const {
                return (_serialJobs.empty() && _lowPriorityJobs.empty() && (_paintJob == 0));
            }
        };

        OpenGLJobProcessor();

        tbb::concurrent_hash_map<tgt::GLCanvas*, PerContextJobQueue*> _contextQueueMap;
        tbb::concurrent_vector<tgt::GLCanvas*> _contexts;

        std::condition_variable _evaluationCondition;   ///< conditional wait to be used when there are currently no jobs to process

        tgt::GLCanvas* _currentContext;         ///< current active OpenGL context
        clock_t _startTimeCurrentContext;       ///<
    };

}

#define GLJobProc tgt::Singleton<OpenGLJobProcessor>::getRef()

#endif // OPENGLJOBPROCESSOR_H__
