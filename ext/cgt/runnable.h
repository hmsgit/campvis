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

#ifndef RUNNABLE_H__
#define RUNNABLE_H__

#include "cgt/logmanager.h"

#include <ext/threading.h>
#include <tbb/atomic.h>

namespace cgt {
    class RunnableWithConditionalWait;

    /**
     * Abstract base class for objects that shall run in a separate thread.
     * Runnable object manage their own thread, which is created when calling start(). The new Thread starts
     * in the run() method which has to be overwritten in subclasses. Calling stop() sets _stopExecution to true
     * and waits for the thread to finish. Hence, you should test for _stopExecution in your run() method.
     */
    class CGT_API Runnable {
        friend void invokeThread(Runnable* r);
        friend class RunnableWithConditionalWait;

    public:
        /**
         * Creates a new Runnable object
         */
        Runnable();

        /**
         * Destructor, stops and waits for the thread to finish if the thread is still running.
         */
        virtual ~Runnable();

        /**
         * Creates the new thread evaluating the run() method.
         * \sa  Runnable::run
         */
        virtual void start();

        /**
         * Sets the _stopExecution flag and waits for the thread to finish.
         */
        virtual void stop();

        /**
         * Entrance point for the new thread. To be overwritten in subclasses.
         */
        virtual void run() = 0;

    protected:
        tbb::atomic<bool> _stopExecution;       ///< Flag whether the thread should stop

    private:
        /// Runnables are not copyable
        explicit Runnable(Runnable const&);
        /// Runnables are not copyable
        Runnable& operator =(Runnable const&);

        std::thread* _thread;                    ///< Thread of the Runnable
        tbb::atomic<bool> _running;
    };



    /**
     * Extension of the Runnable interface for threads that should use conditional wait to pause
     * their work when there's nothing to do.
     * This version adds a protected std::condition_variable to be used for conditional wait and
     * further overloads the stop() method to cleanly halt the thread by repeatedly notifying
     * the condition variable (as notifications may get lost due to race conditions).
     */
    class CGT_API RunnableWithConditionalWait : public Runnable {
    public:
        /**
         * Creates a new RunnableWithConditionalWait object
         */
        RunnableWithConditionalWait();

        /**
         * Destructor, stops and waits for thread to finish if the thread is still running.
         */
        virtual ~RunnableWithConditionalWait();

        /**
         * Sets the _stopExecution flag and waits for the thread to finish.
         */
        virtual void stop();

    protected:
        /// conditional wait to be used when there are currently no jobs to process
        std::condition_variable _evaluationCondition;
    };

}

#endif // RUNNABLE_H__
