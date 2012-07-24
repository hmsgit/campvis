#ifndef RUNNABLE_H__
#define RUNNABLE_H__

#include "tgt/logmanager.h"
#include "tbb/include/tbb/compat/thread"
#include "tbb/include/tbb/atomic.h"

namespace TUMVis {
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

        std::thread _thread;                    ///< Thread of the Runnable
    };

}

#endif // RUNNABLE_H__
