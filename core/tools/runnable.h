// ================================================================================================
// 
// This file is part of the CAMPVis Software Framework.
// 
// If not explicitly stated otherwise: Copyright (C) 2012, all rights reserved,
//      Christian Schulte zu Berge <christian.szb@in.tum.de>
//      Chair for Computer Aided Medical Procedures
//      Technische Universität München
//      Boltzmannstr. 3, 85748 Garching b. München, Germany
// For a full list of authors and contributors, please refer to the file "AUTHORS.txt".
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

#ifndef RUNNABLE_H__
#define RUNNABLE_H__

#include "tgt/logmanager.h"
#include "tbb/compat/thread"
#include "tbb/atomic.h"

namespace campvis {
    /**
     * Abstract base class for objects that shall run in a separate thread.
     * Runnable object manage their own thread, which is created when calling start(). The new Thread starts
     * in the run() method which has to be overwritten in subclasses. Calling stop() sets _stopExecution to true
     * and waits for the thread to finish. Hence, you should test for _stopExecution in your run() method.
     */
    class Runnable {
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
