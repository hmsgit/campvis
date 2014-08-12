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

#ifndef RUNNABLE_H__
#define RUNNABLE_H__

#include "tgt/logmanager.h"

#include <ext/threading.h>
#include <tbb/atomic.h>

namespace tgt {
    /**
     * Abstract base class for objects that shall run in a separate thread.
     * Runnable object manage their own thread, which is created when calling start(). The new Thread starts
     * in the run() method which has to be overwritten in subclasses. Calling stop() sets _stopExecution to true
     * and waits for the thread to finish. Hence, you should test for _stopExecution in your run() method.
     */
    class TGT_API Runnable {
    public:
        /**
         * Creates a new Runnable object
         */
        Runnable();

        /**
         * Destructor, stops and waits the thread if the thread is still running.
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
        Runnable(Runnable const&);
        /// Runnables are not copyable
        Runnable& operator =(Runnable const&);

        std::thread* _thread;                    ///< Thread of the Runnable
        tbb::atomic<bool> _running;
    };

}

#endif // RUNNABLE_H__