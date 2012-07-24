#include "runnable.h"

namespace TUMVis {
    namespace {
        void invokeThread(Runnable* r) {
            r->run();
        }
    }

    Runnable::Runnable() 
        : _stopExecution()
        , _thread() 
    {
        _stopExecution = false;
    }

    Runnable::~Runnable() {
        stop();
    }

    void Runnable::stop() {
        _stopExecution = true; 
        try { 
            if (_thread.joinable())
                _thread.join(); 
        } 
        catch(std::exception& e) { 
            LERRORC("TUMVis.core.tools.Runnable", "Caught exception during _thread.join: " << e.what());
        } 
    }

    void Runnable::start() { 
        _thread = std::thread(&invokeThread, this);
    }
}

