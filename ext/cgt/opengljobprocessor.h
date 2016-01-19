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


#ifndef OPENGLJOBPROCESSOR_H__
#define OPENGLJOBPROCESSOR_H__

#include "cgt/glcontextmanager.h"
#include "cgt/job.h"
#include "cgt/runnable.h"
#include "cgt/singleton.h"
#include "cgt/singleton.h"

#include <ext/threading.h>

#define TBB_PREVIEW_MEMORY_POOL 1
#include <tbb/atomic.h>
#include <tbb/concurrent_queue.h>
#include <tbb/concurrent_hash_map.h>
#include <tbb/concurrent_vector.h>
#include <tbb/memory_pool.h>


namespace cgt {
    class GLCanvas;

    /**
     * This job processor singleton can be used to execute jobs that need an OpenGL context.
     * 
     * Implementing the Runnable interface, the OpenGLJobProcessor runs a background thread having
     * and acquired OpenGL context. You can execute OpenGL calls asynchroniously using enqueueJob()
     * or synchronously using the ScopedSynchronousGlJobExecution guard.
     */
    class CGT_API OpenGLJobProcessor : public cgt::Singleton<OpenGLJobProcessor>, public cgt::RunnableWithConditionalWait {
        friend class cgt::Singleton<OpenGLJobProcessor>;    ///< CRTP
        friend class AbstractJob;                           ///< so the custom new/delete operator can access the memory pool

    public:
        /**
         * Destructor, deletes all unfinished jobs.
         */
        virtual ~OpenGLJobProcessor();


        /**
         * Registers the given OpenGL context, so that it gets its own job queue.
         * \param   context     OpenGL context to register.
         */
        void setContext(cgt::GLCanvas* context);

        /**
         * Returns the OpenGL context of this object..
         * \return  _context
         */
        cgt::GLCanvas* getContext();
        
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

        /**
         * Enqueues the given job.
         * 
         * \note    OpenGLJobProcessor takes ownership of \a job.
         * \param   jfn         Functor defining job to execute.
         */
        void enqueueJob(std::function<void(void)> fn);

        /**
         * Enqueues the given job and blockes the execution until the job has been processed.
         * 
         * \note    OpenGLJobProcessor takes ownership of \a job.
         * \param   fn         Functor defining job to execute.
         */
        void enqueueJobBlocking(std::function<void(void)> fn);


    protected:
        // Protected constructor since it's a singleton
        OpenGLJobProcessor();

        cgt::GLCanvas* _context;                        ///< The OpenGL context to use
        tbb::concurrent_queue<AbstractJob*> _jobQueue;  ///< The OpenGL job queue

        tbb::atomic<int> _pause;                        ///< Counter of pause requests

    private:
        typedef std::allocator<AbstractJob> pool_allocator_t;
        tbb::memory_pool<pool_allocator_t> _jobPool;    ///< Memory pool for the signals
    };

}

#define GLJobProc cgt::Singleton<cgt::OpenGLJobProcessor>::getRef()

#endif // OPENGLJOBPROCESSOR_H__
