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
#include "tbb/include/tbb/compat/condition_variable"
#include "core/tools/jobpool.h"
#include "core/tools/runnable.h"

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
        struct OpenGLJob {
            OpenGLJob(AbstractJob* job, tgt::GLCanvas* canvas)
                : _job(job)
                , _canvas(canvas)
            {}

            AbstractJob* _job;                  ///< Job to execute
            tgt::GLCanvas* _canvas;     ///< OpenGL context to execute job in
        };

        virtual ~OpenGLJobProcessor();


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
        void enqueueJob(tgt::GLCanvas* canvas, AbstractJob* job, PriorityPoolPriority priority);

        /**
         * Slot to be called by _jobPool::s_enqueuedJob signal.
         */
        void OnEnqueuedJob();

    protected:
        OpenGLJobProcessor();

        PriorityPool<OpenGLJob> _jobPool;               ///< PriorityPool to process
        std::condition_variable _evaluationCondition;   ///< conditional wait to be used when there are currently no jobs to process
        tgt::GLCanvas* _currentContext;         ///< current active OpenGL context
    };

}

#define GLJobProc tgt::Singleton<OpenGLJobProcessor>::getRef()

#endif // OPENGLJOBPROCESSOR_H__
