#ifndef RUNNABLE_H__
#define RUNNABLE_H__

#include "tgt/logmanager.h"
#include "tbb/include/tbb/compat/thread"
#include "tbb/include/tbb/atomic.h"

namespace TUMVis {
    /**
     * Abstract base class for objects that shall run in a separate thread.
     */
    class Runnable {
    public:
        Runnable() 
            : _stopExecution()
            , _thread() 
        {}

        virtual ~Runnable() { 
            stop(); 
        }

        virtual void stop() {
            _stopExecution = true; 
            try { 
                _thread.join(); 
            } 
            catch(std::exception& e) { 
                LERRORC("TUMVis.core.tools.Runnable", "Caught exception during _thread.join: " << e.what());
            } 
        }

        void start() { 
            _thread = std::thread(&Runnable::run, *this); 
        }

    protected:
        virtual void run() = 0;
        tbb::atomic<bool> _stopExecution;

    private:
        Runnable(Runnable const&) = delete;
        Runnable& operator =(Runnable const&) = delete;

        std::thread _thread;
    };

}

#endif // RUNNABLE_H__
